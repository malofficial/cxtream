/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_DROP_HPP
#define CXTREAM_DROP_HPP

#include <range/v3/view/transform.hpp>

#include <cxtream/utility/tuple.hpp>

namespace cxtream {


  /* drop */


  template<typename Column>
  constexpr auto drop_fn()
  {
    return ranges::view::transform([](auto&& source) {
      return utility::tuple_remove<Column>(std::forward<decltype(source)>(source));
    });
  }

  // allow calls without parentheses
  template<typename Column>
  auto drop = drop_fn<Column>();


} // end namespace cxtream
#endif
