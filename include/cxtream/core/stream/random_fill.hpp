/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_CORE_STREAM_RANDOM_FILL_HPP
#define CXTREAM_CORE_STREAM_RANDOM_FILL_HPP

#include <cxtream/core/stream/transform.hpp>
#include <cxtream/core/utility/vector.hpp>

#include <range/v3/view/take.hpp>

#include <random>

namespace cxtream::stream {

/// Fill the selected column of a stream with random values.
///
/// This function uses utility::vector::random_fill(). Furthermore, the column to be filled
/// is first resized so that it has the same shape as the selected source column.
///
/// Example:
/// \code
///     CXTREAM_DEFINE_COLUMN(id, int)
///     CXTREAM_DEFINE_COLUMN(category, std::uint64_t)
///     std::vector<int> data = {3, 1, 2};
///     auto rng = data
///       | create<id>()
///       | random_fill(from<id>, to<category>);
///       | transform(from<id, category>, [](...){ ... });
/// \endcode
///
/// \param size_from The column whose size will be used to initialize the random column.
/// \param fill_to The column to be filled with random data.
/// \param ndims The number of random dimensions. See utility::vector::random_fill().
/// \param gen The random generator to be used.
template<typename FromColumn, typename ToColumn, typename Prng = std::mt19937>
constexpr auto random_fill(from_t<FromColumn> size_from,
                           to_t<ToColumn> fill_to,
                           long ndims = std::numeric_limits<long>::max(),
                           Prng&& gen = Prng{std::random_device{}()})
{
    auto fun = [ndims, &gen](const auto& source) -> ToColumn {
        using SourceVector = std::decay_t<decltype(source)>;
        using TargetVector = std::decay_t<decltype(std::declval<ToColumn>().value())>;
        static_assert(utility::ndims<TargetVector>{} <= utility::ndims<SourceVector>{});
        std::vector<std::vector<long>> target_size = utility::ndim_size(source);
        target_size = target_size | ranges::view::take(utility::ndims<TargetVector>{}());
        TargetVector target;
        utility::ndim_resize(target, target_size);
        utility::random_fill(target, ndims, gen);
        return target;
    };
    return transform(from<FromColumn>, to<ToColumn>, std::move(fun), dim<0>);
}

}  // namespace cxtream::stream
#endif
