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
#include <range/v3/algorithm/count.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/zip.hpp>

#include <memory>
#include <tuple>
#include <vector>

using namespace cxtream::stream;

// test with a seeded random generator
std::mt19937 prng{1000003};

CXTREAM_DEFINE_COLUMN(UniqueVec, std::vector<std::unique_ptr<int>>)
CXTREAM_DEFINE_COLUMN(IntVec, std::vector<int>)

auto unique_vec_to_int_vec()
{
    return
        transform(from<UniqueVec>, to<IntVec>, [](auto&& ptrs) {
            return ptrs | ranges::view::indirect;
        }, dim<1>)
      | drop<UniqueVec>;
}

BOOST_AUTO_TEST_CASE(test_partial_transform)
{
    // partial_transform (int){'f', ..., 'i'} to (char){'a', ..., 'd'}
    // the char column is appended
    auto data = ranges::view::iota((int)'f', (int)'j')
      | ranges::view::transform(std::make_tuple<int>)
      | partial_transform(from<int>, to<char>, [](std::tuple<int> t) {
            return std::make_tuple((char)(std::get<0>(t) - 5));
        });

    auto desired =
      ranges::view::zip(ranges::view::iota((char)'a', (char)'e'),
                ranges::view::iota((int)'f', (int)'j'))
      | ranges::view::transform([](auto t) { return std::tuple<char, int>(t); });

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
      | ranges::view::move
      | transform(from<Unique>, to<Unique, Double>,
          [](const std::unique_ptr<int> &ptr) {
            return std::make_tuple(std::make_unique<int>(*ptr), (double)*ptr);
        })
      | ranges::to_vector;

    // check unique pointers
    std::vector<int> desired_ptr_vals{5, 2};
    for (int i = 0; i < 2; ++i) {
        BOOST_TEST(*(std::get<0>(generated[i]).value()[0]) == desired_ptr_vals[i]);
    }

    // check other
    auto to_check = generated | ranges::view::move | drop<Unique>;
    std::vector<std::tuple<Double, Int>> desired = {{5., 3}, {2., 1}};
    test_ranges_equal(to_check, desired);
}

BOOST_AUTO_TEST_CASE(test_mutable)
{
#ifdef CXTREAM_MUTABLE_LAMBDA
    std::vector<std::tuple<Int>> data = {{{1, 3}}, {{5, 7}}};

    auto generated = data
      | ranges::view::move
      | transform(from<Int>, to<Int>, [i = 0](const int&) mutable {
            return i++;
        })
      | ranges::to_vector;

    std::vector<std::tuple<Int>> desired = {{{0, 1}}, {{0, 1}}};
    test_ranges_equal(generated, desired);
#else
    BOOST_TEST_MESSAGE("Cxtream does not support mutable lambdas in this compiler version.");
#endif
}

BOOST_AUTO_TEST_CASE(test_two_to_one)
{
    // transform two columns to a single column
    std::vector<std::tuple<Int, Double>> data = {{{3, 7}, {5., 1.}}, {1, 2.}};
  
    auto generated = data
      | transform(from<Int, Double>, to<Double>, [](int i, double d) {
            return (double)(i + d);
        });
  
    std::vector<std::tuple<Double, Int>> desired = {{{3 + 5., 7 + 1.}, {3, 7}}, {1 + 2., 1}};
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
    std::vector<std::tuple<Int, Double>> data = {{{3, 2}, 5.}, {1, 2.}};
    auto data_orig = data;

    auto generated = data
      | transform(from<Int>, to<Int>, [](const Int& int_batch) {
            std::vector<int> new_batch = int_batch.value();
            new_batch.push_back(4);
            return new_batch;
        }, dim<0>)
      | ranges::to_vector;

    std::vector<std::tuple<Int, Double>> desired = {{{3, 2, 4}, 5.}, {{1, 4}, 2.}};
    BOOST_CHECK(generated == desired);
    BOOST_CHECK(data == data_orig);
}

BOOST_AUTO_TEST_CASE(test_dim2_move_only)
{
    auto data = generate_move_only_data();

    auto rng = data
      | ranges::view::move
      | create<Int, UniqueVec>(2)
      | transform(from<UniqueVec>, to<UniqueVec>, [](std::unique_ptr<int>& ptr) {
            return std::make_unique<int>(*ptr + 1);
        }, dim<2>)
      | drop<Int>
      | unique_vec_to_int_vec();

    std::vector<std::vector<std::vector<int>>> generated = unpack(rng, from<IntVec>, dim<0>);
    std::vector<std::vector<std::vector<int>>> desired = {{{2, 5}, {9, 3}}, {{3, 6}}};
    BOOST_CHECK(generated == desired);
}

BOOST_AUTO_TEST_CASE(test_dim2_move_only_mutable)
{
#ifdef CXTREAM_MUTABLE_LAMBDA
    auto data = generate_move_only_data();

    auto rng = data
      | ranges::view::move
      | create<Int, UniqueVec>(2)
      | transform(from<UniqueVec>, to<UniqueVec>, [i = 4](std::unique_ptr<int>&) mutable {
            return std::make_unique<int>(i++);
        }, dim<2>)
      | drop<Int>
      | unique_vec_to_int_vec();

    std::vector<std::vector<std::vector<int>>> generated = unpack(rng, from<IntVec>, dim<0>);
    std::vector<std::vector<std::vector<int>>> desired = {{{4, 5}, {4, 5}}, {{4, 5}}};
    BOOST_CHECK(generated == desired);
#else
    BOOST_TEST_MESSAGE("Cxtream does not support mutable lambdas in this compiler version.");
#endif
}

BOOST_AUTO_TEST_CASE(test_probabilistic_simple)
{
    CXTREAM_DEFINE_COLUMN(dogs, int)
    std::vector<int> data = {3, 1, 5, 7, 2, 13};
    auto rng = data
      | create<dogs>()
      | transform(from<dogs>, to<dogs>, 1.0, [](int dog) { return 1; }, prng)
      | transform(from<dogs>, to<dogs>, 0.5, [](int dog) { return 2; }, prng)
      | transform(from<dogs>, to<dogs>, 0.0, [](int dog) { return 3; }, prng);

    std::vector<int> generated = unpack(rng, from<dogs>, dim<1>);
    BOOST_CHECK(generated.size() == 6);
    long number1 = ranges::count(generated, 1);
    long number2 = ranges::count(generated, 2);
    long number3 = ranges::count(generated, 3);
    BOOST_TEST(number1 >= 1);
    BOOST_TEST(number1 <= 5);
    BOOST_TEST(number1 == 6 - number2);
    BOOST_TEST(number3 == 0);
}

BOOST_AUTO_TEST_CASE(test_probabilistic_dim2_move_only)
{
    auto data = generate_move_only_data();

    auto rng = data
      | ranges::view::move
      | create<Int, UniqueVec>(2)
      | drop<Int>
      | transform(from<UniqueVec>, to<UniqueVec>, 0.5, [](std::unique_ptr<int>& ptr) {
            return std::make_unique<int>(19);
        }, prng, dim<2>)
      | unique_vec_to_int_vec();

    std::vector<int> generated = unpack(rng, from<IntVec>, dim<2>);
    long number = ranges::count(generated, 19);
    BOOST_TEST(generated.size() == 6);
    BOOST_TEST(number >= 1);
    BOOST_TEST(number <= 5);
}

BOOST_AUTO_TEST_CASE(test_probabilistic_dim2_move_only_multicol)
{
    auto data = generate_move_only_data();

    auto rng = data
      | ranges::view::move
      | create<Int, UniqueVec>(2)
      | drop<Int>
      // create IntVec column
      | transform(from<UniqueVec>, to<IntVec>, [](auto&&) {
            return 7;
        }, dim<2>)
      // probabilistically transform two columns to two columns
      | transform(from<UniqueVec, IntVec>, to<IntVec, UniqueVec>, 0.5,
          [](std::unique_ptr<int>& ptr, int val) {
            return std::make_tuple(val, std::make_unique<int>(19));
        }, prng, dim<2>)
      // probabilistically transform two columns to one column
      | transform(from<IntVec, UniqueVec>, to<UniqueVec>, 0.5,
          [](int, std::unique_ptr<int>& ptr) {
            return std::make_unique<int>(19);
        }, prng, dim<2>)
      | unique_vec_to_int_vec();  // the original IntVec gets overwritten here

    std::vector<int> generated = unpack(rng, from<IntVec>, dim<2>);
    long number19 = ranges::count(generated, 19);
    BOOST_TEST(generated.size() == 6);
    BOOST_TEST(number19 >= 3);
}
