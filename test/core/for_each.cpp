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
#define BOOST_TEST_MODULE for_each_test

#include "common.hpp"

#include <cxtream/core/for_each.hpp>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>

#include <memory>
#include <tuple>
#include <vector>

using namespace cxtream;
using namespace ranges;
using namespace boost;

BOOST_AUTO_TEST_CASE(test_partial_for_each)
{
    // partial_for_each
    std::vector<int> generated;
  
    view::iota(1, 5)
      | view::transform(std::make_tuple<int>)
      | partial_for_each(from<int>, [&generated](const std::tuple<int> &t){
            generated.push_back(std::get<0>(t));
            return 42;
        })
      | to_vector;
  
    test_ranges_equal(generated, view::iota(1, 5));
}

BOOST_AUTO_TEST_CASE(test_partial_for_each_move_only)
{
    // partial_for_each of a move-only column
    std::vector<int> generated;
  
    view::iota(1, 5)
      | view::transform([](int i){
            return std::make_tuple(std::make_unique<int>(i));
        })
      | partial_for_each(from<std::unique_ptr<int>>,
          [&generated](const std::tuple<std::unique_ptr<int>&>& t){
              generated.push_back(*std::get<0>(t));
        })
      | to_vector;
  
    test_ranges_equal(generated, view::iota(1, 5));
}

BOOST_AUTO_TEST_CASE(test_for_each_of_two)
{
    // for_each of two columns
    std::vector<std::tuple<Int, Double>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated = data | for_each(from<Int, Double>, [](const int& v, double c) {});
    std::vector<std::tuple<Int, Double>> desired = {{{3},{5.}}, {{1},{2.}}};
    test_ranges_equal(generated, desired);
}

BOOST_AUTO_TEST_CASE(test_for_each_move_only)
{
    // for_each of a move-only column
    std::vector<std::tuple<Int, Unique>> data;
    data.emplace_back(3, std::make_unique<int>(5));
    data.emplace_back(1, std::make_unique<int>(2));
  
    std::vector<int> generated;
  
    data
      | view::move
      | for_each(from<Int, Unique>,
          [&generated](const int& v, const std::unique_ptr<int>& p){
              generated.push_back(v + *p);
        })
      | to_vector;
  
    test_ranges_equal(generated, std::vector<int>{8, 3});
}
