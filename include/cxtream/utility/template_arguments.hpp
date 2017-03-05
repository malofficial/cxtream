/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_TEMPLATE_ARGUMENTS_HPP
#define CXTREAM_TEMPLATE_ARGUMENTS_HPP

#include <functional>

namespace cxtream {

  using namespace ranges;


  /* helper types */


  template<typename... Columns>
  struct from_t {};

  template<typename... Columns>
  auto from = from_t<Columns...>{};

  template<typename... Columns>
  struct to_t {};

  template<typename... Columns>
  auto to = to_t<Columns...>{};

  template<int Dim>
  struct dim_t {};

  template<int Dim>
  auto dim = dim_t<Dim>{};


  /* helper projections */


  struct identity_t
  {
    template<typename T>
    constexpr T&& operator()(T&& val) const noexcept
    {
      return std::forward<T>(val);
    }
  };

  auto identity = identity_t{};

  struct ref_wrap_t
  {
    template<typename T>
    constexpr decltype(auto) operator()(T& val) const noexcept
    {
      return std::ref(val);
    }
  };

  auto ref_wrap = ref_wrap_t{};


} // end namespace cxtream
#endif
