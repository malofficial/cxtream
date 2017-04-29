/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE utility_vector_test

#include "../common.hpp"

#include <cxtream/core/utility/vector.hpp>

#include <boost/test/unit_test.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/iota.hpp>

#include <memory>
#include <vector>

using namespace cxtream::utility;
using namespace ranges;

BOOST_AUTO_TEST_CASE(test_ndims)
{
    BOOST_TEST(ndims<std::vector<int>>{} == 1);
    BOOST_TEST(ndims<std::vector<std::vector<int>>>{} == 2);
    BOOST_TEST(ndims<std::vector<std::vector<std::vector<int>>>>{} == 3);
}

BOOST_AUTO_TEST_CASE(test_shape)
{
    const std::vector<int> vec5 = {0, 0, 0, 0, 0};
    std::vector<std::vector<int>> vec23 = {{0, 0, 0}, {0, 0, 0}};
    const std::vector<std::vector<std::vector<int>>> vec234 = {
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
    };
    std::vector<std::vector<std::vector<int>>> vec200 = {{}, {}};
    test_ranges_equal(shape(vec5),  std::vector<long>{5});
    test_ranges_equal(shape(vec23), std::vector<long>{2, 3});
    test_ranges_equal(shape(vec234), std::vector<long>{2, 3, 4});
    test_ranges_equal(shape(vec200), std::vector<long>{2, 0, 0});
}

BOOST_AUTO_TEST_CASE(test_flatten)
{
    const std::vector<std::vector<std::vector<int>>> vec = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 15}}
    };
    test_ranges_equal(flat_view(vec), view::iota(1, 16));
}

BOOST_AUTO_TEST_CASE(test_flatten_identity)
{
    const std::vector<int> vec = view::iota(1, 16);
    test_ranges_equal(flat_view(vec), view::iota(1, 16));
}

BOOST_AUTO_TEST_CASE(test_flatten_empty)
{
    std::vector<std::vector<std::vector<int>>> vec = {
        {{}, {}, {}},
        {}
    };
    test_ranges_equal(flat_view(vec), std::vector<int>{});
    std::vector<int> vec2 = {};
    test_ranges_equal(flat_view(vec2), std::vector<int>{});
}

BOOST_AUTO_TEST_CASE(test_flatten_move_only)
{
    std::vector<std::vector<std::unique_ptr<int>>> vec;
    std::vector<std::unique_ptr<int>> inner;
    inner.push_back(std::make_unique<int>(1));
    inner.push_back(std::make_unique<int>(2));
    vec.push_back(std::move(inner));
    inner = std::vector<std::unique_ptr<int>>{};
    inner.push_back(std::make_unique<int>(3));
    inner.push_back(std::make_unique<int>(4));
    vec.push_back(std::move(inner));

    test_ranges_equal(flat_view(vec) | view::indirect, view::iota(1, 5));
}

BOOST_AUTO_TEST_CASE(test_reshape_1d)
{
    std::vector<std::vector<std::vector<int>>> vec = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 15}}
    };
    std::vector<int> rvec = reshaped_view<1>(vec, {15});
    BOOST_TEST(rvec.size() == 15);
    test_ranges_equal(rvec, view::iota(1, 16));
}

BOOST_AUTO_TEST_CASE(test_reshape_2d)
{
    const std::vector<std::vector<std::vector<int>>> vec = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 15}}
    };
    std::vector<std::vector<int>> rvec = reshaped_view<2>(vec, {3, 5});
    BOOST_TEST(rvec.size() == 3);
    for (std::size_t i = 0; i < 3; ++i) {
        BOOST_TEST(rvec[i].size() == 5);
        test_ranges_equal(rvec[i], view::iota(5 * i + 1, 5 * i + 6));
    }
}

BOOST_AUTO_TEST_CASE(test_reshape_3d)
{
    std::vector<std::vector<std::vector<int>>> vec = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 15}}
    };
    std::vector<std::vector<std::vector<int>>> rvec = reshaped_view<3>(vec, {3, 1, 5});
    BOOST_TEST(rvec.size() == 3);
    for (std::size_t i = 0; i < 3; ++i) {
        BOOST_TEST(rvec[i].size() == 1);
        test_ranges_equal(rvec[i][0], view::iota(5 * i + 1, 5 * i + 6));
    }
}

BOOST_AUTO_TEST_CASE(test_reshape_auto_dimension)
{
    const std::vector<std::vector<std::vector<int>>> vec = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 15}}
    };
    std::vector<std::vector<int>> rvec1 = reshaped_view<2>(vec, {3, -1});
    std::vector<std::vector<int>> rvec2 = reshaped_view<2>(vec, {-1, 5});
    BOOST_TEST(rvec1.size() == 3);
    BOOST_TEST(rvec2.size() == 3);
    for (std::size_t i = 0; i < 3; ++i) {
        BOOST_TEST(rvec1[i].size() == 5);
        BOOST_TEST(rvec2[i].size() == 5);
        test_ranges_equal(rvec1[i], view::iota(5 * i + 1, 5 * i + 6));
        test_ranges_equal(rvec2[i], view::iota(5 * i + 1, 5 * i + 6));
    }
}

BOOST_AUTO_TEST_CASE(test_reshape_move_only)
{
    std::vector<std::vector<std::unique_ptr<int>>> vec;
    std::vector<std::unique_ptr<int>> inner;
    inner.push_back(std::make_unique<int>(1));
    inner.push_back(std::make_unique<int>(2));
    vec.push_back(std::move(inner));
    inner = std::vector<std::unique_ptr<int>>{};
    inner.push_back(std::make_unique<int>(3));
    inner.push_back(std::make_unique<int>(4));
    vec.push_back(std::move(inner));

    auto rvec = reshaped_view<2>(vec, {1, 4});
    test_ranges_equal(*ranges::begin(rvec) | view::indirect, view::iota(1, 5));
}
