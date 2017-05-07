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
#include <range/v3/action/sort.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/unique.hpp>

#include <memory>
#include <random>
#include <vector>

using namespace cxtream::utility;
using namespace ranges;

BOOST_AUTO_TEST_CASE(test_ndims)
{
    BOOST_TEST(ndims<std::vector<int>>{} == 1);
    BOOST_TEST(ndims<std::vector<std::vector<int>>>{} == 2);
    BOOST_TEST(ndims<std::vector<std::vector<std::vector<int>>>>{} == 3);
}

BOOST_AUTO_TEST_CASE(test_ndim_type)
{
    static_assert(std::is_same<int,
      ndim_type<std::vector<int>>::type>{});
    static_assert(std::is_same<char,
      ndim_type<std::vector<std::vector<char>>>::type>{});
    static_assert(std::is_same<double,
      ndim_type<std::vector<std::vector<std::vector<double>>>>::type>{});
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

BOOST_AUTO_TEST_CASE(test_ndim_size)
{
    const std::vector<int> vec5 = {0, 0, 0, 0, 0};
    std::vector<std::vector<int>> vec3231 = {{0, 0}, {0, 0, 0}, {0}};
    const std::vector<std::vector<std::vector<int>>> vec3302 = {
        {{0, 0, 0, 0}, {0, 0, 0}, {0, 0}},
        {},
        {{0}, {}}
    };
    std::vector<std::vector<std::vector<int>>> vec200 = {{}, {}};

    test_ranges_equal(ndim_size(vec5),
      std::vector<std::vector<long>>{{5}});
    test_ranges_equal(ndim_size(vec3231),
      std::vector<std::vector<long>>{{3}, {2, 3, 1}});
    test_ranges_equal(ndim_size(vec3302),
      std::vector<std::vector<long>>{{3}, {3, 0, 2}, {4, 3, 2, 1, 0}});
    test_ranges_equal(ndim_size(vec200),
      std::vector<std::vector<long>>{{2}, {0, 0}, {}});
}

BOOST_AUTO_TEST_CASE(test_ndim_resize)
{
    std::vector<std::vector<long>> vec5_size    = {{5}};
    std::vector<std::vector<long>> vec3231_size = {{3}, {2, 3, 1}};
    std::vector<std::vector<long>> vec3302_size = {{3}, {3, 0, 2}, {4, 3, 2, 1, 0}};
    std::vector<std::vector<long>> vec200_size  = {{2}, {0, 0}, {}};

    std::vector<int> vec5_desired = {7, 8, 9, 1, 1};
    std::vector<std::vector<int>> vec3231_desired = {{1, 1}, {3, 4, 2}, {2}};
    std::vector<std::vector<std::vector<int>>> vec3302_desired = {
        {{0, 0, 0, 0}, {0, 0, 0}, {0, 0}},
        {},
        {{0}, {}}
    };
    std::vector<std::vector<std::vector<int>>> vec200_desired = {{}, {}};

    std::vector<int> vec5 = {7, 8, 9};
    std::vector<std::vector<int>> vec3231 = {{1, 1, 1}, {3, 4}};
    std::vector<std::vector<std::vector<int>>> vec3302;
    std::vector<std::vector<std::vector<int>>> vec200;

    BOOST_CHECK(ndim_resize(vec5, vec5_size, 1) == vec5_desired);
    BOOST_CHECK(ndim_resize(vec3231, vec3231_size, 2) == vec3231_desired);
    BOOST_CHECK(ndim_resize(vec3302, vec3302_size) == vec3302_desired);
    BOOST_CHECK(ndim_resize(vec200, vec200_size, 3) == vec200_desired);
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

BOOST_AUTO_TEST_CASE(test_random_fill_1d)
{
    std::mt19937 gen{1000003};
    std::vector<std::uint64_t> vec(10);

    random_fill(vec, 0, gen);
    BOOST_TEST(vec.size() == 10);
    vec |= action::sort;
    auto n_unique = distance(vec | view::unique);
    BOOST_TEST(n_unique == 1);

    random_fill(vec, 5, gen);  // any number larger than 0 should suffice
    BOOST_TEST(vec.size() == 10);
    vec |= action::sort;
    n_unique = distance(vec | view::unique);
    BOOST_TEST(n_unique == 10);
}

BOOST_AUTO_TEST_CASE(test_random_fill_2d)
{
    std::mt19937 gen{1000003};
    std::vector<std::vector<std::uint64_t>> vec = {std::vector<std::uint64_t>(10),
                                                   std::vector<std::uint64_t>(5)};

    auto check = [](auto vec, std::vector<long> unique, long unique_total) {
        for (std::size_t i = 0; i < vec.size(); ++i) {
            vec[i] |= action::sort;
            auto n_unique = distance(vec[i] | view::unique);
            BOOST_TEST(n_unique == unique[i]);
        }

        std::vector<std::uint64_t> all_vals = flat_view(vec);
        all_vals |= action::sort;
        auto n_unique = distance(all_vals | view::unique);
        BOOST_TEST(n_unique == unique_total);
    };

    random_fill(vec, 0, gen);
    check(vec, {1, 1}, 1);
    random_fill(vec, 1, gen);
    check(vec, {1, 1}, 2);
    random_fill(vec, 2, gen);
    check(vec, {10, 5}, 15);
}

BOOST_AUTO_TEST_CASE(test_random_fill_3d)
{
    std::mt19937 gen{1000003};
    std::vector<std::vector<std::vector<std::uint64_t>>> vec =
      {{{0, 0, 0}, {0, 0}, {0}}, {{0}, {0, 0}}};

    auto check = [](auto vec, std::vector<std::vector<long>> unique, long unique_total) {
        for (std::size_t i = 0; i < vec.size(); ++i) {
            for (std::size_t j = 0; j < vec[i].size(); ++j) {
                vec[i][j] |= action::sort;
                auto n_unique = distance(vec[i][j] | view::unique);
                BOOST_TEST(n_unique == unique[i][j]);
            }
        }

        std::vector<std::uint64_t> all_vals = flat_view(vec);
        all_vals |= action::sort;
        auto n_unique = distance(all_vals | view::unique);
        BOOST_TEST(n_unique == unique_total);
    };

    random_fill(vec, 0, gen);
    check(vec, {{1, 1, 1}, {1, 1}}, 1);
    random_fill(vec, 1, gen);
    check(vec, {{1, 1, 1}, {1, 1}}, 2);
    random_fill(vec, 2, gen);
    check(vec, {{1, 1, 1}, {1, 1}}, 5);
    random_fill(vec, 3, gen);
    check(vec, {{3, 2, 1}, {1, 2}}, 9);
}
