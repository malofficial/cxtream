/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transform_test

#include <memory>
#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/zip.hpp>

#include <cxtream/core/drop.hpp>
#include <cxtream/core/transform.hpp>

#include "common.hpp"

using namespace cxtream;
using namespace ranges;
using namespace boost;


BOOST_AUTO_TEST_CASE(test_partial_transform)
{
  // partial_transform (int){'f', ..., 'i'} to (char){'a', ..., 'd'}
  // the char column is appended
  auto data =
      view::iota((int)'f', (int)'j')
    | view::transform(std::make_tuple<int>)
    | partial_transform(from<int>, to<char>, [](std::tuple<int> t){
        return std::make_tuple((char)(std::get<0>(t) - 5));
      });

  auto desired =
      view::zip(view::iota((char)'a', (char)'e'),
                view::iota((int)'f', (int)'j'))
    | view::transform([](auto t){ return std::tuple<char, int>(t); });

  test_ranges_equal(data, desired);
}


BOOST_AUTO_TEST_CASE(test_to_itself)
{
  // transform a single column to itself
  std::vector<std::tuple<Int, Double>> data = {{{3},{5.}}, {{1},{2.}}};

  auto generated =
      data
    | transform(from<Int>, to<Int>, [](const int &v){
        return std::make_tuple(v - 1);
      });

  std::vector<std::tuple<Int, Double>> desired = {{{2},{5.}}, {{0},{2.}}};

  test_ranges_equal(generated, desired);
}


BOOST_AUTO_TEST_CASE(test_move_only)
{
  // transform move-only column
  std::vector<std::tuple<Int, Unique>> data;
  data.emplace_back(3, std::make_unique<int>(5));
  data.emplace_back(1, std::make_unique<int>(2));

  auto generated =
       data
     | view::move
     | transform(from<Unique>, to<Unique, Double>,
         [](const std::unique_ptr<int> &ptr){
           return std::make_tuple(std::make_unique<int>(*ptr), (double)*ptr);
       })
     | to_vector;

  // check unique pointers
  std::vector<int> desired_ptr_vals{5, 2};
  for (int i = 0; i < 2; ++i) {
    BOOST_TEST(*(std::get<0>(generated[i]).value()[0]) == desired_ptr_vals[i]);
  }

  // check other
  auto to_check =
      generated
    | view::move
    | drop<Unique>;

  std::vector<std::tuple<Double, Int>> desired = {{5., 3}, {2., 1}};
  test_ranges_equal(to_check, desired);
}


BOOST_AUTO_TEST_CASE(test_two_to_one)
{
  // transform two columns to a single column
  std::vector<std::tuple<Int, Double>> data = {{{3},{5.}}, {{1},{2.}}};

  auto generated =
      data
    | transform(from<Int, Double>, to<Double>, [](int i, double d){
        return std::make_tuple((double)(i + d));
      });

  std::vector<std::tuple<Double, Int>> desired = {{{3 + 5.}, {3}}, {{1 + 2.}, {1}}};
  test_ranges_equal(generated, desired);
}


BOOST_AUTO_TEST_CASE(test_one_to_two)
{
  // transform a single column to two columns
  std::vector<std::tuple<Int>> data = {{{3}}, {{1}}};

  auto generated =
      data
    | transform(from<Int>, to<Int, Double>, [](int i){
        return std::make_tuple(i + i, (double)(i * i));
      });

  std::vector<std::tuple<Int, Double>> desired = {{{6}, {9.}}, {{2}, {1.}}};
  test_ranges_equal(generated, desired);
}
