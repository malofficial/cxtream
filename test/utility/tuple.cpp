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
#define BOOST_TEST_MODULE utility_tuple_test

#include <boost/test/unit_test.hpp>
#include <cxtream/utility/tuple.hpp>
#include <memory>
#include <vector>
#include <iostream>

using namespace cxtream::utility;


// make the tuple print visible for boost test
// this is forbidden by the standard (simple workarounds?)
namespace std { using cxtream::utility::operator<<; }


BOOST_AUTO_TEST_CASE(utility_tuple_test)
{
  {
    // tuple_contains
    static_assert(tuple_contains<int, std::tuple<>>{} == false);
    static_assert(tuple_contains<int, std::tuple<int>>{} == true);
    static_assert(tuple_contains<int, std::tuple<float>>{} == false);
    static_assert(tuple_contains<int, std::tuple<bool, float, int>>{} == true);
    static_assert(tuple_contains<int, std::tuple<bool, float, char>>{} == false);
    static_assert(tuple_contains<int, std::tuple<const int, const float>>{} == true);
    static_assert(tuple_contains<const int, std::tuple<const int, const float>>{} == true);
  }

  {
    // type_view
    auto t1 = std::make_tuple(0, 5., 'c');
    auto t2 = tuple_type_view<double, int>(t1);
    auto t3 = tuple_type_view<char, int>(t1);
    static_assert(std::is_same<std::tuple<char&, int&>, decltype(t3)>{});
    BOOST_TEST(t2 == std::make_tuple(5., 0));
    BOOST_TEST(t2 == std::make_tuple(5., 0));
    BOOST_TEST(t3 == std::make_tuple('c', 0));
  }

  {
    // type_view writethrough
    auto t1 = std::make_tuple(0, 5., 'c');
    auto t2 = tuple_type_view<double, int>(t1);
    std::get<int&>(t2) = 1;
    BOOST_TEST(std::get<int>(t1) == 1);

    // double writethrough
    auto t3 = tuple_type_view<double&>(t2);
    std::get<double&>(t3) = 3.;
    BOOST_TEST(std::get<double>(t1) == 3);
  }

  {
    // reverse empty
    auto t1 = std::tuple<>{};
    auto t2 = tuple_reverse(t1);
    BOOST_TEST(t2 == std::tuple<>{});
    t2 = std::move(t1);
    BOOST_TEST(t2 == std::tuple<>{});
  }

  {
    // reverse non empty
    auto t1 = std::make_tuple(0, 5., 'c', 3, 'a');
    auto t2 = tuple_reverse(t1);
    static_assert(std::is_same<std::tuple<char, int, char, double, int>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple('a', 3, 'c', 5., 0));
    t2 = tuple_reverse(std::move(t1));
    BOOST_TEST(t2 == std::make_tuple('a', 3, 'c', 5., 0));
  }

  {
    // reverse for references
    double d = 5.;
    char c = 'c';
    int i = 2;
    auto t1 = std::make_tuple(std::ref(i), std::ref(d), std::ref(c));
    auto t2 = tuple_reverse(t1);
    static_assert(std::is_same<std::tuple<char&, double&, int&>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple('c', 5., 2));
    std::get<1>(t2) = 7.;
    BOOST_TEST(d == 7.);
  }

  {
    // reverse move-only
    auto t1 = std::make_tuple(std::unique_ptr<int>{},
                              std::unique_ptr<double>{},
                              std::unique_ptr<bool>{});
    auto t2 = tuple_reverse(std::move(t1));
    static_assert(std::is_same<std::tuple<std::unique_ptr<bool>,
                                          std::unique_ptr<double>,
                                          std::unique_ptr<int>>,
                               decltype(t2)>{});
  }

  {
    // cat_unique add existing type
    auto t1 = std::make_tuple(0, '1');
    auto t2 = tuple_cat_unique(t1, std::make_tuple(1));
    static_assert(std::is_same<std::tuple<int, char>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(0, '1'));
  }

  {
    // cat_unique add multiple existing types
    auto t1 = std::make_tuple(0, '1');
    auto t2 = tuple_cat_unique(std::move(t1), std::make_tuple(2, '3'));
    BOOST_TEST(t2 == std::make_tuple(0, '1'));
  }


  {
    // cat_unique add mix of existing and nonexisting types - rvalue version
    auto t1 = std::make_tuple(0, '2');
    auto t2 = tuple_cat_unique(t1, std::make_tuple('4', 5., 4, 5));
    BOOST_TEST(t2 == std::make_tuple(0, '2', 5.));
  }

  {
    // cat_unique add mix of existing and nonexisting types - lvalue version
    auto a = 5;
    auto b = '4';
    auto c = 5.;
    auto t1 = std::make_tuple(0, '2');
    auto t2 = tuple_cat_unique(std::move(t1), std::make_tuple(b, std::ref(c), 4, a));
    static_assert(std::is_same<std::tuple<int, char, double&>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(0, '2', 5.));
    std::get<2>(t2) = 7.;
    BOOST_TEST(c == 7.);
  }

  {
    // cat_unique move-only
    auto t1 = std::make_tuple(std::unique_ptr<int>{},
                              std::unique_ptr<double>{});
    auto t2 = std::make_tuple(std::unique_ptr<double>{},
                              std::unique_ptr<bool>{},
                              std::unique_ptr<int>{});
    auto t3 = tuple_cat_unique(std::move(t1), std::move(t2));
    static_assert(std::is_same<std::tuple<std::unique_ptr<int>,
                                          std::unique_ptr<double>,
                                          std::unique_ptr<bool>>,
                               decltype(t3)>{});
  }

  {
    // cat_unique const
    double d = 6.;
    const auto t1 = std::make_tuple(4, std::cref(d), 7);
    const auto t2 = std::make_tuple(1., true, 8.);
    auto t3 = tuple_cat_unique(t1, t2);
    static_assert(std::is_same<std::tuple<int, const double&, bool>, decltype(t3)>{});
    BOOST_TEST(t3 == std::make_tuple(4, 6., true));
  }

  {
    // tuple_transform
    auto t1 = std::make_tuple(0, 10L, 5.);
    auto t2 = tuple_transform([](const auto &v){ return v + 1; }, t1);
    static_assert(std::is_same<std::tuple<int, long, double>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(0 + 1, 10L + 1, 5. + 1));
  }

  {
    // tuple_transform with type change
    auto t1 = std::make_tuple(0, 'a', 5.);
    auto t2 = tuple_transform([](auto v){ return 10; }, t1);
    static_assert(std::is_same<std::tuple<int, int, int>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(10, 10, 10));
  }

  {
    // tuple_transform empty
    auto t1 = std::tuple<>{};
    auto t2 = tuple_transform([](auto v){ return v + 1; }, t1);
    static_assert(std::is_same<std::tuple<>, decltype(t2)>{});
    BOOST_TEST(t2 == std::tuple<>{});
  }

  {
    // tuple_transform move-only
    auto t1 = std::make_tuple(std::unique_ptr<int>{},
                              std::unique_ptr<double>{});
    auto t2 = tuple_transform([](auto v){ return v; }, std::move(t1));
    static_assert(std::is_same<std::tuple<std::unique_ptr<int>,
                                          std::unique_ptr<double>>,
                               decltype(t2)>{});
  }

  {
    // tuple_transform for mutable functions
    // beware, the order of application is unspecified
    std::tuple<std::unique_ptr<int>, std::unique_ptr<double>> t1{};

    int called = 0;
    struct Fun
    {
      int& called_;

      std::unique_ptr<int> operator()(std::unique_ptr<int>& ptr)
      {
        called_++;
        return std::make_unique<int>(0);
      }
      std::unique_ptr<double> operator()(std::unique_ptr<double>& ptr)
      {
        called_++;
        return std::make_unique<double>(1);
      }
    } fun{called};

    auto t2 = tuple_transform(fun, t1);
    static_assert(std::is_same<std::tuple<std::unique_ptr<int>,
                                          std::unique_ptr<double>>,
                               decltype(t2)>{});
    BOOST_TEST(called == 2);

    auto t3 = tuple_transform([](const auto &ptr){ return *ptr; }, t2);
    static_assert(std::is_same<std::tuple<int, double>, decltype(t3)>{});
    BOOST_TEST(t3 == std::make_tuple(0, 1.));
  }

  {
    // tuple_for_each
    auto t1 = std::make_tuple(std::make_unique<int>(5), std::make_unique<double>(2.));

    tuple_for_each([](auto& ptr){
        ptr = std::make_unique<std::remove_reference_t<decltype(*ptr)>>(*ptr + 1);
      }, t1);

    auto t2 = tuple_transform([](const auto &ptr){ return *ptr; }, t1);
    static_assert(std::is_same<std::tuple<int, double>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(6, 3.));
  }

  {
    // tuple_for_each
    // assign to pointers increasing values
    std::tuple<std::unique_ptr<int>, std::unique_ptr<double>> t1{};

    struct
    {
      int called = 0;
      void operator()(std::unique_ptr<int>& ptr)
      {
        ptr.reset(new int(called++));
      }
      void operator()(std::unique_ptr<double>& ptr)
      {
        ptr.reset(new double(called++));
      }
    } fun;

    fun = tuple_for_each(fun, t1);
    BOOST_TEST(fun.called == 2);

    auto t2 = tuple_transform([](const auto &ptr){ return *ptr; }, t1);
    static_assert(std::is_same<std::tuple<int, double>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(0, 1.));
  }

  {
    // tuple_remove
    auto t1 = std::make_tuple(5, 10L, 'a');
    auto t2 = tuple_remove<long>(t1);
    static_assert(std::is_same<std::tuple<int, char>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(5, 'a'));
  }

  {
    // tuple_remove not contained
    auto t1 = std::make_tuple(2L, 5L, true);
    auto t2 = tuple_remove<int>(std::move(t1));
    static_assert(std::is_same<std::tuple<long, long, bool>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(2L, 5L, true));
  }

  {
    // tuple_remove multiple
    auto t1 = std::make_tuple(2L, 5L, true);
    auto t2 = tuple_remove<long>(std::move(t1));
    static_assert(std::is_same<std::tuple<bool>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(true));
  }

  {
    // tuple_remove references
    long l1 = 5;
    long l2 = 10L;
    bool b = true;
    auto t1 = std::make_tuple(std::ref(l1), std::ref(l2), std::ref(b));
    auto t2 = tuple_remove<long>(std::move(t1));
    static_assert(std::is_same<std::tuple<bool&>, decltype(t2)>{});
    BOOST_TEST(t2 == std::make_tuple(true));
    std::get<0>(t2) = false;
    BOOST_TEST(b == false);
  }

  {
    // tuple_remove move-only
    auto t1 = std::make_tuple(std::unique_ptr<int>{},
                              std::unique_ptr<bool>{},
                              std::unique_ptr<double>{});
    auto t2 = tuple_remove<std::unique_ptr<bool>>(std::move(t1));
    static_assert(std::is_same<std::tuple<std::unique_ptr<int>,
                                          std::unique_ptr<double>>,
                               decltype(t2)>{});
  }

  {
    // tuple_remove const move-only
    const auto i = std::make_unique<int>(3);
    const auto b = std::make_unique<bool>(true);
    const auto d = std::make_unique<double>(5.);
    const auto t1 = std::make_tuple(std::cref(i), std::cref(b), std::cref(d));
    auto t2 = tuple_remove<std::unique_ptr<bool>>(t1);
    static_assert(std::is_same<std::tuple<const std::unique_ptr<int>&,
                                          const std::unique_ptr<double>&>,
                               decltype(t2)>{});
  }

  {
    // unzip
    std::vector<std::tuple<int, double>> data{};
    data.emplace_back(1, 5.);
    data.emplace_back(2, 6.);
    data.emplace_back(3, 7.);

    std::vector<int> va;
    std::vector<double> vb;
    std::tie(va, vb) = unzip(data);

    std::vector<int> va_desired{1, 2, 3};
    std::vector<double> vb_desired{5., 6., 7.};
    BOOST_TEST(va == va_desired);
    BOOST_TEST(vb == vb_desired);
  }

  {
    // unzip move only
    std::vector<std::tuple<int, std::unique_ptr<int>>> data{};
    data.emplace_back(1, std::make_unique<int>(5));
    data.emplace_back(2, std::make_unique<int>(6));
    data.emplace_back(3, std::make_unique<int>(7));

    std::vector<int> va;
    std::vector<std::unique_ptr<int>> vb;
    std::tie(va, vb) = unzip(std::move(data));

    std::vector<int> va_desired{1, 2, 3};
    BOOST_TEST(va == va_desired);
    for (std::size_t i = 0; i < 3; ++i) {
      BOOST_TEST(*vb[i] == i + 5);
    }
  }
}
