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
#include <range/v3/algorithm/fill.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/for_each.hpp>

#include <cassert>
#include <memory>
#include <random>
#include <type_traits>
#include <vector>

namespace cxtream::utility {

/// Gets the number of dimensions of a multidimensional std::vector type.
///
/// Example:
/// \code
///     std::size_t vec_ndims = ndims<std::vector<std::vector<int>>>{};
///     // vec_ndims == 2;
/// \endcode
template<typename T>
struct ndims {
};

template<typename T>
struct ndims<std::vector<T>> : std::integral_constant<long, 1L> {
};

template<typename T>
struct ndims<std::vector<std::vector<T>>>
  : std::integral_constant<long, ndims<std::vector<T>>{} + 1L> {
};

/// Gets the innermost value_type of a multidimensional std::vector type.
///
/// The number of dimensions can be also provided manually.
///
/// Example:
/// \code
///     using vec_type = ndim_type<std::vector<std::vector<int>>>::type;
///     // vec_type is int
///     using vec1_type = ndim_type<std::vector<std::vector<int>>, 1>::type;
///     // vec1_type is std::vector<int>
/// \endcode
template<typename VecT, long Dim = -1>
struct ndim_type
  : ndim_type<typename VecT::value_type, Dim-1> {
};

template<typename T>
struct ndim_type<T, 0> {
    using type = T;
};

template<typename VecT>
struct ndim_type<VecT, -1>
  : ndim_type<VecT, ndims<VecT>{}> {
};

// multidimensional std::vector size //

namespace detail {

    template<typename T, long Dim>
    struct ndim_size_impl {
    };

    template<typename T, long Dim>
    struct ndim_size_impl<std::vector<T>, Dim> {
        static void impl(const std::vector<T>& vec, std::vector<std::vector<long>>& size_out)
        {
            size_out[Dim].push_back(vec.size());
        }
    };

    template<typename T, long Dim>
    struct ndim_size_impl<std::vector<std::vector<T>>, Dim> {
        static void impl(const std::vector<std::vector<T>>& vec,
                         std::vector<std::vector<long>>& size_out)
        {
            size_out[Dim].push_back(vec.size());
            for (auto& subvec : vec) {
                ndim_size_impl<std::vector<T>, Dim+1>::impl(subvec, size_out);
            }
        }
    };

}  // namespace detail

/// Calculates the size of a multidimensional std::vector.
///
/// i-th element of the resulting vector are the sizes of the vectors in the i-th dimension.
///
/// Example:
/// \code
///     std::vector<std::vector<int>> vec{{1, 2, 3}, {1}, {5, 6}, {7}};
///     std::vector<std::vector<long>> vec_size = ndim_size(vec);
///     // vec_size == {{4}, {3, 1, 2, 1}};
/// \endcode
///
/// \param vec The vector whose size shall be calculated.
/// \returns The sizes of the given vector.
template<typename T>
std::vector<std::vector<long>> ndim_size(const std::vector<T>& vec)
{
    std::vector<std::vector<long>> size_out(ndims<std::vector<T>>{});
    detail::ndim_size_impl<std::vector<T>, 0>::impl(vec, size_out);
    return size_out;
}

// multidimensional std::vector resize //

namespace detail {

    template<typename T, long Dim>
    struct ndim_resize_impl {
    };

    template<typename T, long Dim>
    struct ndim_resize_impl<std::vector<T>, Dim> {
        template<typename ValT>
        static void impl(std::vector<T>& vec,
                         const std::vector<std::vector<long>>& vec_size,
                         std::vector<long>& vec_size_idx,
                         const ValT& val)
        {
            vec.resize(vec_size[Dim][vec_size_idx[Dim]++], val);
        }
    };

    template<typename T, long Dim>
    struct ndim_resize_impl<std::vector<std::vector<T>>, Dim> {
        template<typename ValT>
        static void impl(std::vector<std::vector<T>>& vec,
                         const std::vector<std::vector<long>>& vec_size,
                         std::vector<long>& vec_size_idx,
                         const ValT& val)
        {
            vec.resize(vec_size[Dim][vec_size_idx[Dim]++]);
            for (auto& subvec : vec) {
                ndim_resize_impl<std::vector<T>, Dim+1>::impl(subvec, vec_size, vec_size_idx, val);
            }
        }
    };

}  // namespace detail

/// Resizes a multidimensional std::vector to the given size.
///
/// i-th element of the given size vector are the sizes of the vectors in the i-th dimension.
///
/// Example:
/// \code
///     std::vector<std::vector<int>> vec;
///     ndim_resize(vec, {{2}, {3, 1}}, 2);
///     // vec == {{2, 2, 2}, {2}};
/// \endcode
///
/// \param vec The vector to be resized.
/// \param vec_size The vector to be resized.
/// \param val The value to pad with.
/// \returns The requested size of the given vector.
template<typename T, typename ValT = typename ndim_type<std::vector<T>>::type>
std::vector<T>& ndim_resize(std::vector<T>& vec,
                            const std::vector<std::vector<long>>& vec_size,
                            ValT val = ValT{})
{
    // check that the size is valid
    assert(vec_size.size() == ndims<std::vector<T>>{});
    for (std::size_t i = 1; i < vec_size.size(); ++i) {
        assert(vec_size[i].size() == ranges::accumulate(vec_size[i-1], 0UL));
    }
    // build initial indices
    std::vector<long> vec_size_idx(vec_size.size());
    // recursively resize
    detail::ndim_resize_impl<std::vector<T>, 0>::impl(vec, vec_size, vec_size_idx, val);
    return vec;
}

// multidimensional std::vector shape //

namespace detail {

    template<typename T, long Dim>
    struct shape_impl {
    };

    template<typename T, long Dim>
    struct shape_impl<std::vector<T>, Dim> {
        static void impl(const std::vector<T>& vec, std::vector<long>& shape)
        {
            shape[Dim] = vec.size();
        }
    };

    template<typename T, long Dim>
    struct shape_impl<std::vector<std::vector<T>>, Dim> {
        static void impl(const std::vector<std::vector<T>>& vec, std::vector<long>& shape)
        {
            shape[Dim] = vec.size();
            if (!vec.empty()) shape_impl<std::vector<T>, Dim+1>::impl(vec[0], shape);
        }
    };

    template<typename T, long Dim>
    struct check_shape_impl {
    };

    template<typename T, long Dim>
    struct check_shape_impl<std::vector<T>, Dim> {
        static bool impl(const std::vector<T>& vec, const std::vector<long>& shape)
        {
            return static_cast<long>(vec.size()) == shape[Dim];
        }
    };
    template<typename T, long Dim>
    struct check_shape_impl<std::vector<std::vector<T>>, Dim> {
        static bool impl(const std::vector<std::vector<T>>& vec, const std::vector<long>& shape)
        {
            if (static_cast<long>(vec.size()) != shape[Dim]) return false;
            return ranges::all_of(vec, [&shape](auto& subvec) {
                return check_shape_impl<std::vector<T>, Dim+1>::impl(subvec, shape);
            });
        }
    };

}  // namespace detail

/// Calculates the shape of a multidimensional std::vector.
///
/// \code
///     std::vector<std::vector<int>> vec{{1, 2}, {3, 4}, {5, 6}, {5, 6}};
///     std::vector<long> vec_shape = shape(vec);
///     // vec_shape == {4, 2};
/// \endcode
///
/// \param vec The vector whose shape shall be calculated. All the vectors
///            on the same dimension have to be of equal size.
/// \returns The shape of the given vector.
template<typename T>
std::vector<long> shape(const std::vector<T>& vec)
{
    std::vector<long> shape(ndims<std::vector<T>>{});
    // the ndim_size is not used for efficiency in ndebug mode (only the 0-th element is inspected)
    detail::shape_impl<std::vector<T>, 0>::impl(vec, shape);
    assert((detail::check_shape_impl<std::vector<T>, 0>::impl(vec, shape)));
    return shape;
}

// recursive std::vector flatten //

namespace detail {

    // recursively join the vector
    template<typename VecT, long Dim>
    struct flat_view_impl {
        static auto impl()
        {
            return ranges::view::for_each([](auto& subvec) {
                return subvec | flat_view_impl<VecT, Dim-1>::impl();
            });
        }
    };

    // for 0 and 1, return the original vector
    template<typename VecT>
    struct flat_view_impl<VecT, 1> {
        static auto impl()
        {
            return ranges::view::all;
        }
    };

    template<typename VecT>
    struct flat_view_impl<VecT, 0> : flat_view_impl<VecT, 1> {
    };

    // for -1, the number of dimensions is deduced automatically
    template<typename VecT>
    struct flat_view_impl<VecT, -1> {
        static auto impl()
        {
            return flat_view_impl<VecT, ndims<VecT>{}>::impl();
        }
    };

}  // namespace detail

/// Makes a flat view out of a multidimensional std::vector.
///
/// \code
///     std::vector<std::vector<int>> vec{{1, 2}, {3}, {}, {4, 5, 6}};
///     std::vector<int> rvec = flat_view(vec);
///     // rvec == {1, 2, 3, 4, 5, 6};
///
///     // the same with manually defined number of dimensions
///     std::vector<int> rvec = flat_view<2>(vec);
///     // rvec == {1, 2, 3, 4, 5, 6};
/// \endcode
///
/// \param vec The vector to be flattened.
/// \returns Flat view (InputRange) of the given vector.
template<long NDims = -1, typename T>
auto flat_view(std::vector<T>& vec)
{
    return vec | detail::flat_view_impl<std::vector<T>, NDims>::impl();
}

/// Const version of flat_view.
template<long NDims = -1, typename T>
auto flat_view(const std::vector<T>& vec)
{
    return vec | detail::flat_view_impl<std::vector<T>, NDims>::impl();
}

// std::vector reshape //

namespace detail {

    template<long N>
    struct reshaped_view_impl_go {
        static auto impl(const std::shared_ptr<std::vector<long>>& shape_ptr)
        {
            return ranges::view::chunk((*shape_ptr)[N-2])
              | ranges::view::transform([shape_ptr](auto subview) {
                    return std::move(subview) | reshaped_view_impl_go<N-1>::impl(shape_ptr);
            });
        }
    };

    template<>
    struct reshaped_view_impl_go<1> {
        static auto impl(const std::shared_ptr<std::vector<long>>&)
        {
            return ranges::view::all;
        }
    };

    template<long N, typename Vector>
    auto reshaped_view_impl(Vector& vec, std::vector<long> shape)
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

        // check that all the requested dimenstions have positive size
        assert(ranges::all_of(shape, [](long s) { return s > 0; }));
        // check that the user requests the same number of elements as there really is
        assert(ranges::distance(flat) == ranges::accumulate(shape, 1, std::multiplies<>{}));
        // calculate the cummulative product of the shape list in reverse order
        shape |= ranges::action::reverse;
        ranges::partial_sum(shape, shape, std::multiplies<>{});
        // the recursive chunks will share a single copy of the shape list (performance)
        auto shape_ptr = std::make_shared<std::vector<long>>(std::move(shape));
        return std::move(flat) | detail::reshaped_view_impl_go<N>::impl(shape_ptr);
    }

}  // namespace detail

/// Makes a view of a multidimensional std::vector with a specific shape.
///
/// Usage:
/// \code
///     std::vector<int> vec{1, 2, 3, 4, 5, 6};
///     std::vector<std::vector<int>> rvec = reshaped_view<2>(vec, {2, 3});
///     // rvec == {{1, 2, 3}, {4, 5, 6}};
/// \endcode
///
/// \param vec The base vector for the view.
/// \param shape The list of shapes. Those can contain a single -1, which denotes
///              that the dimension size shall be automatically deduced. All the other values
///              have to be positive.
/// \tparam N The number of dimensions. Has to be equal to shape.size().
/// \returns View (InputRange) of the original vector with the given shape.
template<long N, typename T>
auto reshaped_view(std::vector<T>& vec, std::vector<long> shape)
{
    return detail::reshaped_view_impl<N, std::vector<T>>(vec, std::move(shape));
}

/// Const version of reshaped_view.
template<long N, typename T>
auto reshaped_view(const std::vector<T>& vec, std::vector<long> shape)
{
    return detail::reshaped_view_impl<N, const std::vector<T>>(vec, std::move(shape));
}

// random fill //

namespace detail {

    template<typename T, long Dim>
    struct random_fill_impl {
    };

    template<typename T, long Dim>
    struct random_fill_impl<std::vector<T>, Dim> {
        template<typename Prng>
        static void impl(std::vector<T>& vec, long ndims, Prng& gen)
        {
            if (Dim >= ndims) ranges::fill(vec, gen());
            else for (auto& val : vec) val = gen();
        }
    };

    template<typename T, long Dim>
    struct random_fill_impl<std::vector<std::vector<T>>, Dim> {
        template<typename Prng>
        static void impl(std::vector<std::vector<T>>& vec, long ndims, Prng& gen)
        {
            if (Dim >= ndims) {
                auto val = gen();
                for (auto& elem : flat_view(vec)) elem = val;
            } else {
                for (auto& subvec : vec) {
                    random_fill_impl<std::vector<T>, Dim+1>::impl(subvec, ndims, gen);
                }
            }
        }
    };

}  // namespace detail

/// Fill a multidimensional std::vector with random values.
///
/// If the vector is multidimensional, the random generator may be used only up to the
/// given dimension and the rest of the dimensions will be constant.
///
/// Example:
/// \code
///     std::uniform_int_distribution gen = ...;
///     std::vector<std::vector<std::vector<int>>> data = {{{0, 0, 0},{0}}, {{0}{0, 0}}};
///     random_fill(data, 0, gen);
///     // data == e.g., {{{4, 4, 4},{4}}, {{4}{4, 4}}};
///     random_fill(data, 1, gen);
///     // data == e.g., {{{8, 8, 8},{8}}, {{2}{2, 2}}};
///     random_fill(data, 2, gen);
///     // data == e.g., {{{8, 8, 8},{6}}, {{7}{3, 3}}};
///     random_fill(data, 3, gen);
///     // data == e.g., {{{8, 2, 3},{1}}, {{2}{4, 7}}};
/// \endcode
///
/// \param vec The vector to be filled.
/// \param ndims The random generator will be used only for this number of dimension. The
///              rest of the dimensions will be filled by the last generated value.
/// \param gen The random generator to be used.
template<typename T, typename Prng = std::mt19937>
constexpr void random_fill(std::vector<T>& vec,
                           long ndims = std::numeric_limits<long>::max(),
                           Prng&& gen = Prng{std::random_device{}()})
{
    detail::random_fill_impl<std::vector<T>, 0>::impl(vec, ndims, gen);
}

namespace detail {

    struct same_size_impl {
        template<typename Rng, typename... Rngs>
        constexpr bool operator()(Rng&& rng, Rngs&&... rngs) const
        {
            return ((ranges::size(rng) == ranges::size(rngs)) && ...);
        }

        constexpr bool operator()() const
        {
            return true;
        }
    };

}  // namespace detail

/// Utility function which checks that all the ranges in a tuple have the same size.
///
/// Example:
/// \code
///     std::vector<int> v1 = {1, 2, 3};
///     std::vector<double> v2 = {1., 2., 3.};
///     std::vector<bool> v3 = {true};
///     assert(same_size(std::tie(v1, v2)) == true);
///     assert(same_size(std::tie(v1, v3)) == false);
/// \endcode
///
/// \param rngs A tuple of ranges.
template<typename Tuple>
constexpr bool same_size(Tuple&& rngs)
{
    return std::experimental::apply(detail::same_size_impl{}, rngs);
}

}  // namespace cxtream::utility

#endif
