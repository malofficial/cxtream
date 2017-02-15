/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE buffered_view_test

#include <tuple>
#include <vector>
#include <ostream>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/generate_n.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>
#include <view/buffered.hpp>


using namespace stream;
using namespace boost;


BOOST_AUTO_TEST_CASE(buffered_view_test)
{
  {
    // check simple traverse
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto rng = view::buffered_view(data, std::launch::async, 2);
    auto it = ranges::begin(rng);
    static_assert(std::is_same<int, decltype(*it)>{});
    BOOST_TEST(*it == 1);

    auto generated = rng | ranges::to_vector;

    BOOST_TEST(*it == 1);
    BOOST_TEST(generated == data, test_tools::per_element{});
  }
}
