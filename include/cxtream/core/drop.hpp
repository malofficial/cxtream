/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_DROP_HPP
#define CXTREAM_CORE_DROP_HPP

#include <cxtream/core/utility/tuple.hpp>

#include <range/v3/view/transform.hpp>

namespace cxtream {

template<typename... Columns>
constexpr auto drop_fn()
{
    static_assert(sizeof...(Columns) == 1, "Dropping multiple columns simultaneously is not"
                                           " implemented yet. Please drop the columns one by one.");

    return ranges::view::transform([](auto&& source) {
        return utility::tuple_remove<Columns...>(std::forward<decltype(source)>(source));
    });
}

/// Drops columns from a range of tuples of columns.
///
/// Example:
/// \code
///     CXTREAM_DEFINE_COLUMN(id, int)
///     auto rng = ranges::view::iota(0, 10) | cxtream::create<id> | cxtream::drop<id>;
///     // rng is a range of empty tuples
/// \endcode
template <typename... Columns>
auto drop = drop_fn<Columns...>();

}  // end namespace cxtream
#endif
