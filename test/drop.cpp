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
#define BOOST_TEST_MODULE column_transform_test

#include <memory>
#include <ostream>
#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/move.hpp>

#include <cxtream/column.hpp>
#include <cxtream/drop.hpp>

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


BOOST_AUTO_TEST_CASE(transform_test)
{

  {
    // drop column
    std::vector<std::tuple<A, B>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | drop<A>
      | to_vector;

    std::vector<std::tuple<B>> desired = {{{5.}}, {{2.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  // TODO drop move-only

}
