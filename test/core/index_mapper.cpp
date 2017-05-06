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
    std::vector<std::string> vals = {"second", "first", "third"};
    std::vector<std::size_t> idxs = {1UL, 0UL, 2UL};
    test_ranges_equal(mapper.index_for(vals), idxs);
    test_ranges_equal(mapper.at(idxs), vals);
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
