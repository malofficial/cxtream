/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE filter_test

#include "../common.hpp"

#include <cxtream/core/stream/create.hpp>
#include <cxtream/core/stream/filter.hpp>
#include <cxtream/core/stream/for_each.hpp>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/move.hpp>

#include <tuple>
#include <vector>

using namespace cxtream::stream;

BOOST_AUTO_TEST_CASE(test_dim0)
{
    const std::vector<std::tuple<int, double>> data = {
      {{3, 5.}, {1, 2.}, {7, 3.}, {8, 1.}, {2, 4.}, {6, 5.}}};

    std::size_t i = 0;
    data
      | create<Int, Double>(2)
      // for dim0, `from` is ignored anyway
      | filter(from<Int, Double>, by<Double>,
          [](const std::vector<double>& v) { return v.at(0) > 3.; }, dim<0>)
      | for_each(from<Int, Double>, [&i](auto& ints, auto& doubles) {
            switch (i++) {
            case 0: BOOST_TEST(ints    == (std::vector<int>{3, 1}));
                    BOOST_TEST(doubles == (std::vector<double>{5., 2.}));
                    break;
            case 1: BOOST_TEST(ints    == (std::vector<int>{2, 6}));
                    BOOST_TEST(doubles == (std::vector<double>{4., 5.}));
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 2);
}

BOOST_AUTO_TEST_CASE(test_dim0_move_only)
{
    std::vector<std::tuple<Int, Unique>> data;
    data.emplace_back(3, std::make_unique<int>(5));
    data.emplace_back(1, std::make_unique<int>(2));

    std::size_t i = 0;
    data
      | ranges::view::move
      // for dim0, `from` is ignored anyway
      | filter(from<>, by<Unique>,
          [](const std::vector<std::unique_ptr<int>>& v) { return *(v.at(0)) >= 3; }, dim<0>)
      | for_each(from<Int, Unique>, [&i](auto& ints, auto& uniques) {
            switch (i++) {
            case 0: BOOST_TEST(ints == (std::vector<int>{3}));
                    BOOST_TEST(uniques.size() == 1);
                    BOOST_TEST(*(uniques.at(0)) == 5);
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 1);
}

BOOST_AUTO_TEST_CASE(test_dim1)
{
    const std::vector<std::tuple<int, double>> data = {
      {{3, 5.}, {1, 2.}, {2, 4.}, {6, 5.}}};

    std::size_t i = 0;
    data
      | create<Int, Double>(2)
      | filter(from<Int, Double>, by<Double>, [](double v) { return v >= 5.; }, dim<1>)
      | for_each(from<Int, Double>, [&i](auto& ints, auto& doubles) {
            switch (i++) {
            case 0: BOOST_TEST(ints    == (std::vector<int>{3}));
                    BOOST_TEST(doubles == (std::vector<double>{5.}));
                    break;
            case 1: BOOST_TEST(ints    == (std::vector<int>{6}));
                    BOOST_TEST(doubles == (std::vector<double>{5.}));
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 2);
}

BOOST_AUTO_TEST_CASE(test_dim1_partial)
{
    const std::vector<std::tuple<int, double>> data = {
      {{3, 5.}, {1, 2.}, {2, 4.}, {6, 5.}}};

    std::size_t i = 0;
    data
      | create<Int, Double>(2)
      | filter(from<Double>, by<Double>, [](double v) { return v >= 5.; }, dim<1>)
      | for_each(from<Int, Double>, [&i](auto& a, auto& b) {
            switch (i++) {
            case 0: BOOST_CHECK(a == (std::vector<int>{3, 1}));
                    BOOST_CHECK(b == (std::vector<double>{5.}));
                    break;
            case 1: BOOST_CHECK(a == (std::vector<int>{2, 6}));
                    BOOST_CHECK(b == (std::vector<double>{5.}));
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 2);
}

BOOST_AUTO_TEST_CASE(test_dim1_move_only)
{
    std::vector<std::tuple<Int, Unique>> data;
    data.emplace_back(3, std::make_unique<int>(5));
    data.emplace_back(1, std::make_unique<int>(2));
    data.emplace_back(2, std::make_unique<int>(4));
    data.emplace_back(6, std::make_unique<int>(5));

    std::size_t i = 0;
    data
      | ranges::view::move
      | filter(from<Unique>, by<Unique>, [](auto& ptr) { return *ptr >= 5.; }, dim<1>)
      | for_each(from<Int, Unique>, [&i](auto& a, auto& b) {
            switch (i++) {
            case 0: BOOST_CHECK(a == (std::vector<int>{3}));
                    BOOST_TEST(b.size() == 1);
                    BOOST_CHECK(*(b.at(0)) == 5.);
                    break;
            case 1: BOOST_CHECK(a == (std::vector<int>{1}));
                    BOOST_TEST(b.size() == 0);
                    break;
            case 2: BOOST_CHECK(a == (std::vector<int>{2}));
                    BOOST_TEST(b.size() == 0);
                    break;
            case 3: BOOST_CHECK(a == (std::vector<int>{6}));
                    BOOST_TEST(b.size() == 1);
                    BOOST_CHECK(*(b.at(0)) == 5.);
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 4);
}

BOOST_AUTO_TEST_CASE(test_dim2)
{
    CXTREAM_DEFINE_COLUMN(IntVec1, std::vector<int>)
    CXTREAM_DEFINE_COLUMN(IntVec2, std::vector<int>)
    const std::vector<std::tuple<std::vector<int>, std::vector<int>>> data = {
      {{{3, 2}, {1, 5}}, {{1, 5}, {2, 4}}, {{2, 4}, {7, 1}}, {{6, 4}, {3, 5}}}};

    std::size_t i = 0;
    auto generated = data
      | create<IntVec1, IntVec2>(2)
      | filter(from<IntVec1, IntVec2>, by<IntVec2>, [](int v) { return v >= 4; }, dim<2>)
      | for_each(from<IntVec1, IntVec2>, [&i](auto& iv1, auto& iv2) {
            switch (i++) {
            case 0: BOOST_TEST(iv1 == (std::vector<std::vector<int>>{{2}, {5}}));
                    BOOST_TEST(iv2 == (std::vector<std::vector<int>>{{5}, {4}}));
                    break;
            case 1: BOOST_TEST(iv1 == (std::vector<std::vector<int>>{{2}, {4}}));
                    BOOST_TEST(iv2 == (std::vector<std::vector<int>>{{7}, {5}}));
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 2);
}

BOOST_AUTO_TEST_CASE(test_dim2_partial)
{
    CXTREAM_DEFINE_COLUMN(IntVec, std::vector<int>)
    const std::vector<std::tuple<int, std::vector<int>>> data = {
      {{3, {1, 5}}, {1, {2, 4}}, {2, {7, 1}}, {6, {3, 5}}}};

    std::size_t i = 0;
    auto generated = data
      | create<Int, IntVec>(2)
      | filter(from<IntVec>, by<IntVec>, [](int v) { return v >= 4; }, dim<2>)
      | for_each(from<Int, IntVec>, [&i](auto& ints, auto& intvecs) {
            switch (i++) {
            case 0: BOOST_TEST(ints    == (std::vector<int>{3, 1}));
                    BOOST_TEST(intvecs == (std::vector<std::vector<int>>{{5}, {4}}));
                    break;
            case 1: BOOST_TEST(ints    == (std::vector<int>{2, 6}));
                    BOOST_TEST(intvecs == (std::vector<std::vector<int>>{{7}, {5}}));
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 2);
}

BOOST_AUTO_TEST_CASE(test_dim2_move_only)
{
    CXTREAM_DEFINE_COLUMN(UniqueVec, std::vector<std::unique_ptr<int>>)
    std::vector<std::tuple<UniqueVec>> data;
    std::vector<std::unique_ptr<int>> v1;
    std::vector<std::unique_ptr<int>> v2;
    std::vector<std::unique_ptr<int>> v3;
    v1.emplace_back(std::make_unique<int>(5));
    v1.emplace_back(std::make_unique<int>(3));
    v2.emplace_back(std::make_unique<int>(2));
    v2.emplace_back(std::make_unique<int>(4));
    v3.emplace_back(std::make_unique<int>(1));
    v3.emplace_back(std::make_unique<int>(6));
    data.emplace_back(std::move(v1));
    data.emplace_back(std::move(v2));
    data.emplace_back(std::move(v3));

    std::size_t i = 0;
    data
      | ranges::view::move
      | filter(from<UniqueVec>, by<UniqueVec>, [](auto& ptr) { return *ptr >= 4.; }, dim<2>)
      | for_each(from<UniqueVec>, [&i](auto& unique_vec) {
            switch (i++) {
            BOOST_TEST(unique_vec.size() == 1);
            BOOST_TEST(unique_vec.at(0).size() == 1);
            case 0: BOOST_TEST(*(unique_vec.at(0).at(0)) == 5);
                    break;
            case 1: BOOST_TEST(*(unique_vec.at(0).at(0)) == 4);
                    break;
            case 2: BOOST_TEST(*(unique_vec.at(0).at(0)) == 6);
                    break;
            }
        }, dim<0>)
      | ranges::to_vector;
    BOOST_TEST(i == 3);
}
