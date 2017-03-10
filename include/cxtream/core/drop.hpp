/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_CORE_DROP_HPP
#define CXTREAM_CORE_DROP_HPP

#include <range/v3/view/transform.hpp>

#include <cxtream/core/utility/tuple.hpp>

namespace cxtream {


  /* drop */


  template<typename... Columns>
  constexpr auto drop_fn()
  {
    static_assert(sizeof...(Columns) == 1, "Dropping multiple columns simultaneously is not"
      " implemented yet. Please drop the columns one by one.");

    return ranges::view::transform([](auto&& source) {
      return utility::tuple_remove<Columns...>(std::forward<decltype(source)>(source));
    });
  }

  // allow calls without parentheses
  template<typename... Columns>
  auto drop = drop_fn<Columns...>();


} // end namespace cxtream
#endif
