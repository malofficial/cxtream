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
#define BOOST_TEST_MODULE column_create_test

#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>

#include <cxtream/create.hpp>

#include "common.hpp"

using namespace cxtream;
using namespace ranges;
using namespace boost;


BOOST_AUTO_TEST_CASE(create_test)
{
  {
    // create a new column
    auto generated =
        view::iota(0, 10)
      | create<Int>
      | to_vector;

    std::vector<std::tuple<Int>> desired = view::iota(0, 10);

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  // TODO create move-only

}
