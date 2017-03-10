/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_COLUMN_HPP
#define CXTREAM_CORE_COLUMN_HPP

#include <type_traits>
#include <vector>

namespace cxtream {


  /* column definition macro */


  template<typename T, bool = std::is_copy_constructible<T>{}>
  struct column_base
  {
    std::vector<T> value;

    column_base() = default;
    column_base(T&& rhs)
    {
      value.emplace_back(std::move(rhs));
    }
    column_base(const T& rhs)
      : value{rhs}
    { }
    column_base(std::vector<T>&& rhs)
      : value{std::move(rhs)}
    { }
    column_base(const std::vector<T>& rhs)
      : value{rhs}
    { }
  };

  template<typename T>
  struct column_base<T, false> : column_base<T, true>
  {
    using column_base<T, true>::column_base;

    column_base() = default;
    column_base(const T& rhs) = delete;
    column_base(const std::vector<T>& rhs) = delete;
  };

} // end namespace cxtream


#define CXTREAM_DEFINE_COLUMN(col_name, col_type) \
struct col_name : cxtream::column_base<col_type> { \
  using cxtream::column_base<col_type>::column_base; \
  static constexpr const char* name = #col_name; \
};


#endif
