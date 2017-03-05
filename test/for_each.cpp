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
#define BOOST_TEST_MODULE for_each_test

#include <memory>
#include <ostream>
#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>

#include <cxtream/column.hpp>
#include <cxtream/for_each.hpp>

// make the tuple print visible for boost test
// this is forbidden by the standard (simple workarounds?)
namespace std { using cxtream::utility::operator<<; }

using namespace cxtream;
using namespace ranges;
using namespace boost;
using namespace utility;

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
{
  out << "{";
  for (std::size_t i = 0; i < vec.size(); ++i) {
    out << vec[i];
    if (i + 1 != vec.size())
      out << ", ";
  }
  out << "}";
  return out;
}


inline namespace columns {
  CXTREAM_DEFINE_COLUMN(A, int)
  CXTREAM_DEFINE_COLUMN(B, double)
  CXTREAM_DEFINE_COLUMN(Unique, std::unique_ptr<int>)
  CXTREAM_DEFINE_COLUMN(Shared, std::shared_ptr<int>)
}


bool operator==(const A& lhs, const A& rhs)
{ return lhs.value == rhs.value; }
bool operator==(const B& lhs, const B& rhs)
{ return lhs.value == rhs.value; }
std::ostream& operator<<(std::ostream& out, const A& rhs)
{ return out << rhs.value; }
std::ostream& operator<<(std::ostream& out, const B& rhs)
{ return out << rhs.value; }


BOOST_AUTO_TEST_CASE(for_each_test)
{
  {
    // partial_for_each
    std::vector<int> generated;

      view::iota(1, 5)
    | view::transform(std::make_tuple<int>)
    | partial_for_each(from<int>, [&generated](const std::tuple<int> &t){
        generated.push_back(std::get<0>(t));
        return 42;
      })
    | to_vector;

    auto desired = view::iota(1, 5) | to_vector;

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // partial_for_each of a move-only column
    std::vector<int> generated;

      view::iota(1, 5)
    | view::transform([](int i){
        return std::make_tuple(std::make_unique<int>(i));
      })
    | partial_for_each(from<std::unique_ptr<int>>,
        [&generated](const std::tuple<std::unique_ptr<int>&>& t){
          generated.push_back(*std::get<0>(t));
      })
    | to_vector;

    auto desired = view::iota(1, 5) | to_vector;

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // for_each of two columns
    std::vector<std::tuple<A, B>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | for_each(from<A, B>, [](const int &v, char c){ })
      | to_vector;

    std::vector<std::tuple<A, B>> desired = {{{3},{5.}}, {{1},{2.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // for_each of a move-only column
    std::vector<std::tuple<A, Unique>> data;
    data.emplace_back(3, std::make_unique<int>(5));
    data.emplace_back(1, std::make_unique<int>(2));

    std::vector<int> generated;

      data
    | view::move
    | for_each(from<A, Unique>,
        [&generated](const int& v, const std::unique_ptr<int>& p){
          generated.push_back(v + *p);
      })
    | to_vector;

    std::vector<int> desired = {8, 3};
    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

}
