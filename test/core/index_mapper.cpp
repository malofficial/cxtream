/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE index_mapper_test

#include "common.hpp"

#include <cxtream/core/index_mapper.hpp>

#include <boost/test/unit_test.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/cycle.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/take.hpp>

#include <stdexcept>

using namespace ranges;
using namespace cxtream;
using namespace boost;

BOOST_AUTO_TEST_CASE(test_construction)
{
    const index_mapper<std::string> mapper{{"first", "second", "third"}};
    test_ranges_equal(mapper.values(), std::vector<std::string>{"first", "second", "third"});
    BOOST_TEST(mapper.size() == 3UL);
}

BOOST_AUTO_TEST_CASE(test_mapping)
{
    const index_mapper<std::string> mapper{{"first", "second", "third"}};
    BOOST_TEST(mapper.index_for("second") == 1UL);
    BOOST_TEST(mapper.index_for("third") == 2UL);
    BOOST_TEST(mapper.index_for("first") == 0UL);
    BOOST_TEST(mapper.index_for("fourth", 10UL) == 10UL);
    BOOST_CHECK_THROW(mapper.index_for("fourth"), std::out_of_range);
    BOOST_TEST(mapper.at(2UL) == "third");
    BOOST_TEST(mapper.at(1UL) == "second");
    BOOST_TEST(mapper.at(0UL) == "first");
    BOOST_TEST(mapper.contains("first"));
    BOOST_TEST(mapper.contains("second"));
    BOOST_TEST(!mapper.contains("fourth"));
}

BOOST_AUTO_TEST_CASE(test_multi_mapping)
{
    const index_mapper<std::string> mapper{{"first", "second", "third"}};
    // value to index - only existing
    std::vector<std::string> vals = {"second", "first", "third"};
    std::vector<std::size_t> idxs = {1UL, 0UL, 2UL};
    test_ranges_equal(mapper.index_for(vals), idxs);
    // index to value
    test_ranges_equal(mapper.at(idxs), vals);
    // value to index - also non-existing with default value
    vals = {"second", "bogus", "first", "third", "fourth"};
    idxs = {1UL, 10UL, 0UL, 2UL, 10UL};
    test_ranges_equal(mapper.index_for(vals, 10UL), idxs);
    // value to index - also non-existing
    BOOST_CHECK_THROW(mapper.index_for(vals), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(test_insertion)
{
    index_mapper<std::string> mapper{{"first", "second", "third"}};
    mapper.insert("fourth");
    test_ranges_equal(mapper.values(),
                      std::vector<std::string>{"first", "second", "third", "fourth"});
    BOOST_TEST(mapper.at(3UL) == "fourth");
    BOOST_TEST(mapper.index_for("fourth") == 3UL);
    BOOST_TEST(mapper.size() == 4UL);
}

BOOST_AUTO_TEST_CASE(test_try_insertion)
{
    index_mapper<std::string> mapper{{"first", "second", "third"}};
    BOOST_TEST(mapper.try_insert("first") == false);
    BOOST_TEST(mapper.try_insert("fourth") == true);
    BOOST_TEST(mapper.try_insert("second") == false);
    test_ranges_equal(mapper.values(),
                      std::vector<std::string>{"first", "second", "third", "fourth"});
}

BOOST_AUTO_TEST_CASE(test_make_unique_index_mapper_container)
{
    std::vector<std::string> data = {"bum", "bada", "bum", "bum", "bada", "yeah!"};
    index_mapper<std::string> mapper = make_unique_index_mapper(data);
    test_ranges_equal(mapper.values(), std::vector<std::string>{"bum", "bada", "yeah!"});
}

BOOST_AUTO_TEST_CASE(test_make_unique_index_mapper_view)
{
    // test view of a vector
    std::vector<int> data = {3, 2, 3, 1, 1, 2, 2, 1};
    index_mapper<int> mapper = make_unique_index_mapper(ranges::view::all(data));
    test_ranges_equal(mapper.values(), std::vector<int>{3, 2, 1});

    // test pure view
    mapper = make_unique_index_mapper(ranges::view::iota(2, 5)
                                        | ranges::view::cycle
                                        | ranges::view::take(10));
    test_ranges_equal(mapper.values(), std::vector<int>{2, 3, 4});
}
