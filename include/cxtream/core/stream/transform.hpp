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

#include <cxtream/build_config.hpp>
#include <cxtream/core/stream/template_arguments.hpp>
#include <cxtream/core/utility/random.hpp>
#include <cxtream/core/utility/tuple.hpp>
#include <cxtream/core/utility/vector.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <functional>
#include <utility>

namespace cxtream::stream {

/// Transform a subset of tuple elements for each tuple in a range and concatenate the result
/// with the source tuple.
///
/// The result tuple overrides the corresponding types from the source tuple.
template<typename... FromTypes, typename... ToTypes,
         typename Fun, typename Projection = ref_wrap_t>
constexpr auto partial_transform(from_t<FromTypes...>, to_t<ToTypes...>,
                                 Fun fun, Projection proj = Projection{})
{
    static_assert(sizeof...(ToTypes) > 0, "For non-transforming operations, please"
                                          " use the partial_for_each.");

    return ranges::view::transform([fun = std::move(fun), proj = std::move(proj)]
      (auto&& source) CXTREAM_MUTABLE_LAMBDA_V {
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
    template<std::size_t Dim, std::size_t NOuts>
    struct wrap_fun_for_dim
    {
        template<typename Fun>
        static constexpr auto impl(Fun fun)
        {
            return [fun = std::move(fun)](auto&& tuple_of_ranges) CXTREAM_MUTABLE_LAMBDA_V {
                assert(utility::same_size(tuple_of_ranges));
                auto range_of_tuples =
                  std::experimental::apply(ranges::view::zip,
                                           std::forward<decltype(tuple_of_ranges)>(tuple_of_ranges))
                  | ranges::view::transform(wrap_fun_for_dim<Dim-1, NOuts>::impl(fun));
                return utility::unzip_if<(NOuts > 1)>(std::move(range_of_tuples));
            };
        }
    };

    template<std::size_t NOuts>
    struct wrap_fun_for_dim<0, NOuts>
    {
        template<typename Fun>
        static constexpr auto impl(Fun fun)
        {
            return [fun = std::move(fun)](auto&& tuple) CXTREAM_MUTABLE_LAMBDA_V {
                return std::experimental::apply(fun, std::forward<decltype(tuple)>(tuple));
            };
        }
    };

}  // namespace detail

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
/// \param fun The function to be applied. The function should return the type represented
///            by the target column in the given dimension. If there are multiple target
///            columns, the function should return a tuple of the corresponding types.
/// \param d The dimension in which the function is applied. Choose 0 for the function to
///          be applied to the whole batch.
template<typename... FromColumns, typename... ToColumns, typename Fun, int Dim = 1>
constexpr auto transform(from_t<FromColumns...> f,
                         to_t<ToColumns...> t,
                         Fun fun,
                         dim_t<Dim> d = dim_t<1>{})
{
    // wrap the function to be applied in the appropriate dimension
    auto fun_wrapper = detail::wrap_fun_for_dim<Dim, sizeof...(ToColumns)>::impl(std::move(fun));

    return partial_transform(f, t, std::move(fun_wrapper),
                             [](auto& column) { return std::ref(column.value()); });
}

// probabilistic transform //

namespace detail {

    // This function accepts a tuple of references and returns a new tuple
    // made by moving the values from the original tuple.
    // If the tuple is of size 1, then it only returns the rvalue
    // of its element (without wrapping it in a tuple).
    // Furthermore, one can specify which elements should be taken
    // using std::index_sequence.
    template<std::size_t NArgs>
    struct move_to_maybe_make_tuple
    {
        template<typename Tuple, std::size_t... Is>
        static constexpr auto impl(Tuple& tuple, std::index_sequence<Is...>)
        {
            return std::make_tuple(std::move(std::get<Is>(tuple))...);
        }
    };

    template<>
    struct move_to_maybe_make_tuple<1>
    {
        template<typename Tuple, std::size_t I>
        static constexpr auto impl(Tuple& tuple, std::index_sequence<I>)
        {
            return std::move(std::get<I>(tuple));
        }
    };

    // wrap the function to be an identity if the dice roll fails
    template<std::size_t NOuts>
    struct wrap_fun_with_prob
    {
        template<typename Fun, typename Prng, typename ToIndices>
        static constexpr auto impl(
          double prob,
          Fun fun,
          Prng& prng,
          ToIndices to_indices)
        {
            return [fun = std::move(fun), &prng, prob, to_indices]
              (auto&... cols) CXTREAM_MUTABLE_LAMBDA_V {
                std::uniform_real_distribution<> dis(0, 1);
                // apply the function if the dice roll succeeds
                if (dis(prng) < prob) {
                    return std::invoke(fun, std::forward<decltype(cols)>(cols)...);
                // return the original arguments if the dice roll fails
                } else {
                    auto args_view = std::forward_as_tuple<decltype(cols)...>(cols...);
                    // Note: We can force std::move in here, because
                    // we are only copying data to themselves.
                    return move_to_maybe_make_tuple<NOuts>::impl(args_view, to_indices);
                }
            };
        }
    };

}  // namespace detail

/// Probabilistic transform of a subset of cxtream columns.
///
/// This function behaves the same as the original stream::transform, but it accepts one extra
/// argument denoting the probability of transformation. If this probability is 0.0,
/// the transformer behaves as an identity. If it is 1.0, the transofrmation function
/// is always applied.
///
/// Example:
/// \code
///     CXTREAM_DEFINE_COLUMN(dogs, int)
///     std::vector<std::tuple<int>> data = {3, 1, 5, 7};
///     auto rng = data
///       | create<dogs>()
///       // In 50% of the cases, the number of dogs increase,
///       // and in the other 50% of the cases, it stays the same.
///       | transform(from<dogs>, to<dogs>, 0.5, [](int dog) { return dog + 1; });
/// \endcode
///
/// \param f The columns to be extracted out of the tuple of columns and passed to fun.
/// \param t The columns where the result will be saved. This has to be a subset of f.
/// \param prob The probability of transformation. If the dice roll fails, the transformer
///             applies an identity on the target columns.
/// \param fun The function to be applied. The function should return the type represented
///            by the selected column in the given dimension. If there are multiple target
///            columns, the function should return a tuple of the corresponding types.
/// \param d The dimension in which the function is applied. Choose 0 for the function to
///          be applied to the whole batch.
/// \param prng The random generator to be used. Defaults to a thread_local
///             std::mt19937.
template<
  typename... FromColumns,
  typename... ToColumns,
  typename Fun,
  typename Prng = std::mt19937,
  int Dim = 1>
constexpr auto transform(
  from_t<FromColumns...> f,
  to_t<ToColumns...> t,
  double prob,
  Fun fun,
  Prng& prng = utility::random_generator,
  dim_t<Dim> d = dim_t<1>{})
{
    // find out which indexes in FromColumns... are for ToColumns...
    std::index_sequence<utility::variadic_find<ToColumns, FromColumns...>::value...> to_indices;

    // wrap the function to be applied in the appropriate dimension with the given probabiliy
    auto prob_fun = detail::wrap_fun_with_prob<sizeof...(ToColumns)>::impl(
      prob, std::move(fun), prng, to_indices);

    return transform(f, t, std::move(prob_fun), d);
}

} // namespace cxtream::stream
#endif
