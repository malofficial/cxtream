/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_CREATE_HPP
#define CXTREAM_CREATE_HPP

#include <range/v3/view/transform.hpp>

#include <cxtream/utility/tuple.hpp>

namespace cxtream {


  /* create */


  template<typename Column>
  constexpr auto create_fn()
  {
    return ranges::view::transform([](auto&& source){
      return std::tuple<Column>{std::forward<decltype(source)>(source)};
    });
  }

  // allow calls without parentheses
  template<typename Column>
  auto create = create_fn<Column>();


} // end namespace cxtream
#endif
