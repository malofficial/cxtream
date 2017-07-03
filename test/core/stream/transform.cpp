/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transform_test

#include "../common.hpp"

#include <cxtream/core/stream/create.hpp>
#include <cxtream/core/stream/drop.hpp>
#include <cxtream/core/stream/transform.hpp>
#include <cxtream/core/stream/unpack.hpp>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/zip.hpp>

#include <memory>
#include <tuple>
#include <vector>

using namespace cxtream::stream;
using namespace ranges;
using namespace boost;

CXTREAM_DEFINE_COLUMN(UniqueVec, std::vector<std::unique_ptr<int>>)
CXTREAM_DEFINE_COLUMN(IntVec, std::vector<int>)

BOOST_AUTO_TEST_CASE(test_partial_transform)
{
    // partial_transform (int){'f', ..., 'i'} to (char){'a', ..., 'd'}
    // the char column is appended
    auto data = view::iota((int)'f', (int)'j')
      | view::transform(std::make_tuple<int>)
      | partial_transform(from<int>, to<char>, [](std::tuple<int> t) {
            return std::make_tuple((char)(std::get<0>(t) - 5));
        });

    auto desired =
      view::zip(view::iota((char)'a', (char)'e'),
                view::iota((int)'f', (int)'j'))
      | view::transform([](auto t) { return std::tuple<char, int>(t); });

    test_ranges_equal(data, desired);
}

BOOST_AUTO_TEST_CASE(test_to_itself)
{
    // transform a single column to itself
    std::vector<std::tuple<Int, Double>> data = {{3, 5.}, {1, 2.}};
    auto generated = data
      | transform(from<Int>, to<Int>, [](const int& v) { return v - 1; });
    std::vector<std::tuple<Int, Double>> desired = {{2, 5.}, {0, 2.}};
    test_ranges_equal(generated, desired);
}

BOOST_AUTO_TEST_CASE(test_move_only)
{
    // transform move-only column
    std::vector<std::tuple<Int, Unique>> data;
    data.emplace_back(3, std::make_unique<int>(5));
    data.emplace_back(1, std::make_unique<int>(2));

    auto generated = data
      | view::move
      | transform(from<Unique>, to<Unique, Double>,
          [](const std::unique_ptr<int> &ptr) {
            return std::make_tuple(std::make_unique<int>(*ptr), (double)*ptr);
        })
      | to_vector;

    // check unique pointers
    std::vector<int> desired_ptr_vals{5, 2};
    for (int i = 0; i < 2; ++i) {
        BOOST_TEST(*(std::get<0>(generated[i]).value()[0]) == desired_ptr_vals[i]);
    }

    // check other
    auto to_check = generated | view::move | drop<Unique>;
    std::vector<std::tuple<Double, Int>> desired = {{5., 3}, {2., 1}};
    test_ranges_equal(to_check, desired);
}

BOOST_AUTO_TEST_CASE(test_two_to_one)
{
    // transform two columns to a single column
    std::vector<std::tuple<Int, Double>> data = {{3, 5.}, {1, 2.}};
  
    auto generated = data
      | transform(from<Int, Double>, to<Double>, [](int i, double d) {
            return (double)(i + d);
        });
  
    std::vector<std::tuple<Double, Int>> desired = {{3 + 5., 3}, {1 + 2., 1}};
    test_ranges_equal(generated, desired);
}

BOOST_AUTO_TEST_CASE(test_one_to_two)
{
    // transform a single column to two columns
    std::vector<std::tuple<Int>> data = {{{3}}, {{1}}};
  
    auto generated = data
      | transform(from<Int>, to<Int, Double>, [](int i) {
            return std::make_tuple(i + i, (double)(i * i));
        });
  
    std::vector<std::tuple<Int, Double>> desired = {{6, 9.}, {2, 1.}};
    test_ranges_equal(generated, desired);
}

BOOST_AUTO_TEST_CASE(test_dim0)
{
    std::vector<std::tuple<Int, Double>> data = {{3, 5.}, {1, 2.}};
    auto data_orig = data;

    auto generated = data
      | transform(from<Int>, to<Int>, [](const Int& int_batch) {
            std::vector<int> new_batch = int_batch.value();
            new_batch.push_back(4);
            return new_batch;
        }, dim<0>)
      | to_vector;

    std::vector<std::tuple<Int, Double>> desired = {{{3, 4}, 5.}, {{1, 4}, 2.}};
    BOOST_CHECK(generated == desired);
    BOOST_CHECK(data == data_orig);
}

BOOST_AUTO_TEST_CASE(test_dim2_move_only)
{
    auto data = generate_move_only_data();

    auto rng = data
      | view::move
      | create<Int, UniqueVec>(2)
      | transform(from<UniqueVec>, to<UniqueVec>, [](std::unique_ptr<int>& ptr) {
            (*ptr)++;
            return std::move(ptr);
        }, dim<2>)
      | drop<Int>
      | transform(from<UniqueVec>, to<IntVec>, [](auto&& ptrs) {
            return ptrs | view::indirect;
        }, dim<1>)
      | drop<UniqueVec>;

    std::vector<std::vector<std::vector<int>>> generated = unpack(rng, from<IntVec>, dim<0>);
    std::vector<std::vector<std::vector<int>>> desired = {{{2, 5}, {9, 3}}, {{3, 6}}};
    BOOST_CHECK(generated == desired);
}
