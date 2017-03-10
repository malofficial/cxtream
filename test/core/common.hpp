/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Doubleoost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include <memory>
#include <ostream>
#include <vector>

#include <cxtream/core/column.hpp>


// make the tuple print visible for boost test
// this is forbidden by the standard (simple workarounds?)
namespace std { using cxtream::utility::operator<<; }


inline namespace columns {
  CXTREAM_DEFINE_COLUMN(Int, int)
  CXTREAM_DEFINE_COLUMN(Double, double)
  CXTREAM_DEFINE_COLUMN(Unique, std::unique_ptr<int>)
  CXTREAM_DEFINE_COLUMN(Shared, std::shared_ptr<int>)
}


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


bool operator==(const Int& lhs, const Int& rhs)
{ return lhs.value == rhs.value; }
bool operator==(const Double& lhs, const Double& rhs)
{ return lhs.value == rhs.value; }
std::ostream& operator<<(std::ostream& out, const Int& rhs)
{ return out << rhs.value; }
std::ostream& operator<<(std::ostream& out, const Double& rhs)
{ return out << rhs.value; }

#endif
