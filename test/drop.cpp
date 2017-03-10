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
#define BOOST_TEST_MODULE column_drop_test

#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/move.hpp>

#include <cxtream/drop.hpp>

#include "common.hpp"

using namespace cxtream;
using namespace ranges;
using namespace boost;


CXTREAM_DEFINE_COLUMN(Unique2, std::unique_ptr<int>)


BOOST_AUTO_TEST_CASE(drop_test)
{
  {
    // drop column
    std::vector<std::tuple<Int, Double>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | drop<Int>
      | to_vector;

    std::vector<std::tuple<Double>> desired = {{{5.}}, {{2.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // drop a move-only column
    std::vector<std::tuple<Unique, Unique2>> data;
    data.emplace_back(std::make_unique<int>(1), std::make_unique<int>(5));
    data.emplace_back(std::make_unique<int>(2), std::make_unique<int>(6));

    auto generated = 
        data
      | view::move
      | drop<Unique>
      | view::transform([](auto t){ return *(std::get<0>(std::move(t)).value[0]); })
      | to_vector;

    std::vector<int> desired = {5, 6};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    /*
    // drop multiple columns
    std::vector<std::tuple<Int, Unique, Unique2>> data;
    data.emplace_back(-1, std::make_unique<int>(1), std::make_unique<int>(5));
    data.emplace_back(-2, std::make_unique<int>(2), std::make_unique<int>(6));

    auto generated = 
        data
      | view::move
      | drop<Int, Unique>
      | view::transform([](auto t){ return *(std::get<0>(std::move(t)).value[0]); })
      | to_vector;

    std::vector<int> desired = {5, 6};

    BOOST_TEST(generated == desired, test_tools::per_element{});
    */
  }
}