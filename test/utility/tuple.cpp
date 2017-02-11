/// \file
// Tuple utilities
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE utility_tuple


#include <boost/test/unit_test.hpp>
#include <utility/tuple.hpp>


using namespace stream::utility;
using namespace std;


struct A {
  int value;
  bool operator==(const A& rhs) const
  {
      return value == rhs.value; 
  }
};
struct B {
  int value;
  bool operator==(const B& rhs) const
  {
      return value == rhs.value; 
  }
};
struct C {
  int value;
  bool operator==(const C& rhs) const
  {
      return value == rhs.value; 
  }
};


BOOST_AUTO_TEST_CASE(utility_tuple)
{
  {
    // tuple_contains
    static_assert(tuple_contains<int, std::tuple<>>{} == false);
    static_assert(tuple_contains<int, std::tuple<int>>{} == true);
    static_assert(tuple_contains<int, std::tuple<float>>{} == false);
    static_assert(tuple_contains<int, std::tuple<bool, float, int>>{} == true);
    static_assert(tuple_contains<int, std::tuple<bool, float, char>>{} == false);
    static_assert(tuple_contains<int, std::tuple<const int, const float>>{} == false);
    static_assert(tuple_contains<const int, std::tuple<const int, const float>>{} == true);
  }

  {
    // type_view
    auto t1 = std::make_tuple(0, 5., 'c');
    auto t2 = tuple_type_view<double, int>(t1);
    auto t3 = tuple_type_view<char, int>(t1);
    static_assert(std::is_same<std::tuple<char&, int&>, decltype(t3)>{});
    BOOST_CHECK(t2 == std::make_tuple(5., 0));
    BOOST_CHECK(t2 == std::make_tuple(5., 0));
    BOOST_CHECK(t3 == std::make_tuple('c', 0));
  }

  {
    // type_view writethrough
    auto t1 = std::make_tuple(0, 5., 'c');
    auto t2 = tuple_type_view<double, int>(t1);
    std::get<int&>(t2) = 1;
    BOOST_CHECK(std::get<int>(t1) == 1);

    // double writethrough
    auto t3 = tuple_type_view<double&>(t2);
    std::get<double&>(t3) = 3.;
    BOOST_CHECK(std::get<double>(t1) == 3);
  }

  {
    // reverse empty
    auto t1 = std::tuple<>{};
    auto t2 = tuple_reverse(t1);
    BOOST_CHECK(t2 == std::tuple<>{});
    t2 = std::move(t1);
    BOOST_CHECK(t2 == std::tuple<>{});
  }

  {
    // reverse non empty
    auto t1 = std::make_tuple(0, 5., 'c', 3, 'a');
    auto t2 = tuple_reverse(t1);
    static_assert(std::is_same<std::tuple<char, int, char, double, int>, decltype(t2)>{});
    BOOST_CHECK(t2 == std::make_tuple('a', 3, 'c', 5., 0));
    t2 = tuple_reverse(std::move(t1));
    BOOST_CHECK(t2 == std::make_tuple('a', 3, 'c', 5., 0));
  }

  {
    // cat_unique add existing type
    auto t1 = std::make_tuple(A{0}, B{1});
    auto t2 = tuple_cat_unique(std::move(t1), std::make_tuple(A{1}));
    static_assert(std::is_same<std::tuple<A, B>, decltype(t2)>{});
    BOOST_CHECK(t2 == std::make_tuple(A{0}, B{1}));
  }

  {
    // cat_unique add multiple existing types
    auto t1 = std::make_tuple(A{0}, B{1});
    auto t2 = tuple_cat_unique(std::move(t1), std::make_tuple(A{2}, B{3}));
    BOOST_CHECK(t2 == std::make_tuple(A{0}, B{1}));
  }


  {
    // cat_unique add mix of existing and nonexisting types - rvalue version
    auto t1 = std::make_tuple(A{0}, B{2});
    auto t2 = tuple_cat_unique(std::move(t1), std::make_tuple(B{4}, C{5}, A{4}, A{5}));
    BOOST_CHECK(t2 == std::make_tuple(A{0}, B{2}, C{5}));
  }

  {
    // cat_unique add mix of existing and nonexisting types - lvalue version
    auto a = A{5};
    auto b = B{4};
    auto c = C{5};
    auto t1 = std::make_tuple(A{0}, B{2});
    auto t2 = tuple_cat_unique(std::move(t1), std::make_tuple(b, c, A{4}, a));
    BOOST_CHECK(t2 == std::make_tuple(A{0}, B{2}, C{5}));
  }
}
