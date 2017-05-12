/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_CORE_STREAM_TRANSFORM_HPP
#define CXTREAM_CORE_STREAM_TRANSFORM_HPP

#include <cxtream/core/stream/template_arguments.hpp>
#include <cxtream/core/utility/tuple.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <functional>
#include <utility>

namespace cxtream::stream {

/// Transform a subset of tuple elements for each tuple in a range.
template<typename... FromTypes, typename... ToTypes,
         typename Fun, typename Projection = ref_wrap_t>
constexpr auto partial_transform(from_t<FromTypes...>, to_t<ToTypes...>,
                                 Fun fun, Projection proj = Projection{})
{
    static_assert(sizeof...(ToTypes) > 0, "For non-transforming operations, please"
                                          " use the partial_for_each.");

    return ranges::view::transform([fun = std::move(fun), proj = std::move(proj)]
      (auto&& source) mutable {
        // build the view for the transformer, i.e., slice and project
        auto slice_view =
          utility::tuple_transform(proj, utility::tuple_type_view<FromTypes...>(source));
        // process the transformer's result and convert it to the requested types
        std::tuple<ToTypes...> result{std::invoke(fun, std::move(slice_view))};
        // replace the corresponding fields
        return utility::tuple_cat_unique(std::move(result), std::forward<decltype(source)>(source));
    });
}

namespace detail {

    // apply fun to each element in tuple of ranges in the given dimension
    template<int Dim>
    struct wrap_fun_for_dim
    {
        template<typename Fun>
        static constexpr auto impl(Fun fun)
        {
            return [fun = std::move(fun)](auto&& tuple_of_ranges) {
                auto range_of_tuples =
                  std::experimental::apply(ranges::view::zip,
                                           std::forward<decltype(tuple_of_ranges)>(tuple_of_ranges))
                  | ranges::view::transform(wrap_fun_for_dim<Dim - 1>::impl(fun));
                return utility::unzip(std::move(range_of_tuples));
            };
        }
    };

    template<>
    struct wrap_fun_for_dim<0>
    {
        template<typename Fun>
        static constexpr auto impl(Fun fun)
        {
            return [fun = std::move(fun)](auto&& tuple) {
                return std::experimental::apply(fun, std::forward<decltype(tuple)>(tuple));
            };
        }
    };

} // namespace detail

/// Transform a subset of cxtream columns to a different subset of cxtream columns.
///
/// Example:
/// \code
///     CXTREAM_DEFINE_COLUMN(id, int)
///     CXTREAM_DEFINE_COLUMN(value, double)
///     std::vector<std::tuple<int, double>> data = {{3, 5.}, {1, 2.}};
///     auto rng = data
///       | create<id, value>()
///       | transform(from<id>, to<value>, [](int id) { return id * 5. + 1.; });
/// \endcode
///
/// \param f The columns to be extracted out of the tuple of columns and passed to fun.
/// \param t The columns where the result will be saved. If the stream does not contain
///          the selected columns, they are added to the stream. This parameter can
///          overlap with the parameter f.
/// \param fun The function to be applied.
/// \param d The dimension in which the function is applied. Choose 0 for the function to
///          be applied to the whole batch.
template<typename... FromColumns, typename... ToColumns, typename Fun, int Dim = 1>
constexpr auto transform(from_t<FromColumns...> f,
                         to_t<ToColumns...> t,
                         Fun fun,
                         dim_t<Dim> d = dim_t<1>{})
{
    // wrap the function to be applied in the appropriate dimension
    auto fun_wrapper = detail::wrap_fun_for_dim<Dim>::impl(std::move(fun));

    return partial_transform(f, t, std::move(fun_wrapper),
                             [](auto& column) { return std::ref(column.value()); });
}

} // namespace cxtream::stream
#endif
