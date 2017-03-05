/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE for_each_test

#include <memory>
#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>

#include <cxtream/for_each.hpp>

#include "common.hpp"

using namespace cxtream;
using namespace ranges;
using namespace boost;


BOOST_AUTO_TEST_CASE(for_each_test)
{
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

    auto desired = view::iota(1, 5) | to_vector;

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

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

    auto desired = view::iota(1, 5) | to_vector;

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // for_each of two columns
    std::vector<std::tuple<Int, Double>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | for_each(from<Int, Double>, [](const int &v, char c){ })
      | to_vector;

    std::vector<std::tuple<Int, Double>> desired = {{{3},{5.}}, {{1},{2.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

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

    std::vector<int> desired = {8, 3};
    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

}
