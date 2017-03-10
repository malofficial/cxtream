/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_CREATE_HPP
#define CXTREAM_CORE_CREATE_HPP

#include <range/v3/view/transform.hpp>

#include <cxtream/core/utility/tuple.hpp>

namespace cxtream {


  /* create */


  template<typename... Columns>
  constexpr auto create_fn()
  {
    return ranges::view::transform([](auto&& source){
      return std::tuple<Columns...>{std::forward<decltype(source)>(source)};
    });
  }

  // allow calls without parentheses
  template<typename... Columns>
  auto create = create_fn<Columns...>();


} // end namespace cxtream
#endif
