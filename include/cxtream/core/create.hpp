/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_CORE_CREATE_HPP
#define CXTREAM_CORE_CREATE_HPP

#include <cxtream/core/utility/tuple.hpp>

#include <range/v3/view/transform.hpp>

namespace cxtream {

template<typename... Columns>
constexpr auto create_fn()
{
  return ranges::view::transform([](auto&& source) {
      return std::tuple<Columns...>{std::forward<decltype(source)>(source)};
  });
}

/// Converts a range to a range of tuples of columns.
///
/// Example:
/// \code
///     CXTREAM_DEFINE_COLUMN(id, int)
///     auto rng = ranges::view::iota(0, 10) | cxtream::create<id>;
/// \endcode
template<typename... Columns>
auto create = create_fn<Columns...>();

} // end namespace cxtream
#endif
