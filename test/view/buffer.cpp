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
#define BOOST_TEST_MODULE buffer_view_test

#include <vector>
#include <memory>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/indirect.hpp>

#include <view/buffer.hpp>


using namespace stream;
using namespace boost;
using namespace std::chrono_literals;


void test_use_count(const std::vector<std::shared_ptr<int>>& ptrs,
                    const std::vector<int>& desired)
{
  for (unsigned i = 0; i < ptrs.size(); ++i)
    BOOST_TEST(ptrs[i].use_count() == desired[i]);
}


BOOST_AUTO_TEST_CASE(buffer_view_test)
{
  {
    // check simple traverse
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto rng1 = buffer(data, 2);
    auto rng2 = data | buffer(2);

    auto it1 = ranges::begin(rng1);
    static_assert(std::is_same<int, decltype(*it1)>{});
    BOOST_TEST(*it1 == 1);

    auto it2 = ranges::begin(rng2);
    static_assert(std::is_same<int, decltype(*it2)>{});
    BOOST_TEST(*it2 == 1);

    auto generated1 = rng1 | ranges::to_vector;
    auto generated2 = rng2 | ranges::to_vector;

    BOOST_TEST(generated1 == data, test_tools::per_element{});
    BOOST_TEST(generated2 == data, test_tools::per_element{});
  }

  {
    // check if it is really buffer
    std::vector<std::shared_ptr<int>> data;
    for (int i = 0; i < 5; ++i)
      data.emplace_back(std::make_shared<int>(i));
    auto rng = buffer(data, 2);
    BOOST_TEST(rng.size() == data.size());

    // iterate through and check not-yet visited elements' use count
    test_use_count(data, {1, 1, 1, 1, 1});
    auto it = ranges::begin(rng);
    BOOST_CHECK(it != ranges::end(rng));
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {2, 2, 1, 1, 1});
    ++it;
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {1, 2, 2, 1, 1});
    ++it;
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {1, 1, 2, 2, 1});
    ++it;
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {1, 1, 1, 2, 2});
    ++it;
    BOOST_CHECK(it != ranges::end(rng));
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {1, 1, 1, 1, 2});
    ++it;
    BOOST_CHECK(it == ranges::end(rng));
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {1, 1, 1, 1, 1});

    // iterate with two iterators at once
    auto it2 = ranges::begin(rng);
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {2, 2, 1, 1, 1});
    auto it3 = ranges::begin(rng);
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {3, 3, 1, 1, 1});
    ++it2;
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {2, 3, 2, 1, 1});
    static_assert(std::is_same<std::shared_ptr<int>, decltype(*it)>{});

    // check values
    auto vals = rng | ranges::view::indirect | ranges::to_vector;
    auto desired = ranges::view::iota(0, 5) | ranges::to_vector;
    BOOST_TEST(vals == desired, test_tools::per_element{});
  }

  {
    // check infinite size buffer
    std::vector<std::shared_ptr<int>> data;
    for (int i = 0; i < 5; ++i)
      data.emplace_back(std::make_shared<int>(i));
    auto rng = data | buffer;
    BOOST_TEST(rng.size() == data.size());

    // iterate through and check not-yet visited elements' use count
    test_use_count(data, {1, 1, 1, 1, 1});
    auto it = ranges::begin(rng);
    BOOST_CHECK(it != ranges::end(rng));
    std::this_thread::sleep_for(40ms);
    test_use_count(data, {2, 2, 2, 2, 2});
    ++it;
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {1, 2, 2, 2, 2});
    ++it; ++it; ++it; ++it;
    BOOST_CHECK(it == ranges::end(rng));
    std::this_thread::sleep_for(20ms);
    test_use_count(data, {1, 1, 1, 1, 1});
  }
}
