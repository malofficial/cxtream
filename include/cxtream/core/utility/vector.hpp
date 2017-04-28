/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_CORE_VECTOR_UTILS_HPP
#define CXTREAM_CORE_VECTOR_UTILS_HPP

#include <range/v3/action/reverse.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/for_each.hpp>

#include <memory>
#include <vector>

namespace cxtream::utility {

// recursive std::vector flatten //

namespace detail {

    template<typename T>
    struct flat_view_impl {
    };

    template<typename T>
    struct flat_view_impl<std::vector<T>> {
        static auto impl(std::vector<T>& vec)
        {
            return vec | ranges::view::all;
        }
    };

    template<typename T>
    struct flat_view_impl<std::vector<std::vector<T>>> {
        static auto impl(std::vector<std::vector<T>>& vec)
        {
            return vec
              | ranges::view::for_each([](std::vector<T>& subvec) {
                    return flat_view_impl<std::vector<T>>::impl(subvec);
            });
        }
    };

}  // namespace detail

/// Make a flat view out of a multidimensional std::vector.
///
/// \code
///     std::vector<std::vector<int>> vec{{1, 2}, {3}, {}, {4, 5, 6}};
///     std::vector<int> rvec = flat_view(vec);
///     // rvec == {1, 2, 3, 4, 5, 6};
/// \endcode
///
/// \param vec The vector to be flattened.
/// \returns Flat view (InputRange) of the given vector.
template<typename T>
auto flat_view(std::vector<T>& vec)
{
    return detail::flat_view_impl<std::vector<T>>::impl(vec);
}

// std::vector reshape //

namespace detail {

    template<long N>
    struct reshaped_view_impl {
        static auto impl(const std::shared_ptr<std::vector<long>>& shape_ptr)
        {
            return ranges::view::chunk((*shape_ptr)[N-2])
              | ranges::view::transform([shape_ptr](auto subview) {
                    return std::move(subview) | reshaped_view_impl<N-1>::impl(shape_ptr);
            });
        }
    };

    template<>
    struct reshaped_view_impl<1> {
        static auto impl(const std::shared_ptr<std::vector<long>>&)
        {
            return ranges::view::all;
        }
    };

}  // namespace detail

/// Make a multidimensional view of a multidimensional std::vector, reshaping the dimensions.
///
/// Usage:
/// \code
///     std::vector<int> vec{1, 2, 3, 4, 5, 6};
///     std::vector<std::vector<int>> rvec = reshaped_view<2>(vec, {2, 3});
///     // rvec == {{1, 2, 3}, {4, 5, 6}};
/// \endcode
///
/// \param vec The vector to reshape.
/// \param shape The list of shapes. There can be a single -1, which denotes
///              automatically deduced dimension shape. All the other values
///              have to be positive.
/// \tparam N The number of dimensions. Has to be equal to shape.size().
/// \returns View (InputRange) of the original vector with the given shape.
template<long N, typename T>
auto reshaped_view(std::vector<T>& vec, std::vector<long> shape)
{
    assert(shape.size() == N);
    auto flat = flat_view(vec);

    // if -1 present in the shape list, deduce the dimension
    auto deduced_pos = ranges::find(shape, -1);
    if (deduced_pos != shape.end()) {
        auto flat_size = ranges::distance(flat);
        auto shape_prod = -ranges::accumulate(shape, 1, std::multiplies<>{});
        assert(flat_size % shape_prod == 0);
        *deduced_pos = flat_size / shape_prod;
    }

    // check that all the dimenstions have positive size
    assert(ranges::all_of(shape, [](long s) { return s > 0; }));
    
    // check that the user requests the same number of elements as there really is
    assert(ranges::distance(flat) == ranges::accumulate(shape, 1, std::multiplies<>{}));
    // calculate the cummulative product of the shape list in reverse order
    shape |= ranges::action::reverse;
    ranges::partial_sum(shape, shape, std::multiplies<>{});
    // the recursive chunks will share a single copy of the shape list
    auto shape_ptr = std::make_shared<std::vector<long>>(std::move(shape));
    return std::move(flat) | detail::reshaped_view_impl<N>::impl(shape_ptr);
}

}  // namespace cxtream::utility

#endif
