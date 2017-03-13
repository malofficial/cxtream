/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_FOR_EACH_HPP
#define CXTREAM_CORE_FOR_EACH_HPP

#include <utility>
#include <functional>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <cxtream/core/utility/tuple.hpp>
#include <cxtream/core/utility/template_arguments.hpp>

namespace cxtream {


  /* partial_for_each */


  template<typename... FromTypes, typename Fun, typename Projection = ref_wrap_t>
  constexpr auto partial_for_each(from_t<FromTypes...>,
                                  Fun fun, Projection proj = Projection{})
  {
    return ranges::view::transform([fun=std::move(fun), proj=std::move(proj)]
      (auto&& source) mutable {
        // build the view for the transformer, i.e., slice and project
        const auto slice_view =
          utility::tuple_transform(proj, utility::tuple_type_view<FromTypes...>(source));
        // apply the function
        std::invoke(fun, slice_view);
        // return the original
        return std::forward<decltype(source)>(source);
    });
  }


  /* for_each */


  namespace detail {

    // apply fun to some dimension
    template<int Dim>
    struct wrap_void_fun_for_dim
    {
      template<typename Fun>
      static constexpr auto impl(Fun fun)
      {
        return [fun=std::move(fun)](auto&& tuple_of_ranges){
          auto range_of_tuples =
              std::experimental::apply(
                  ranges::view::zip,
                  std::forward<decltype(tuple_of_ranges)>(tuple_of_ranges));

          for (auto&& tuple : range_of_tuples) {
            wrap_void_fun_for_dim<Dim - 1>::impl(fun)(
              std::forward<decltype(tuple)>(tuple));
          }
        };
      }
    };

    // apply fun to the entire batch
    template<>
    struct wrap_void_fun_for_dim<0>
    {
      template<typename Fun>
      static constexpr auto impl(Fun fun)
      {
        return [fun=std::move(fun)](auto&& tuple){
          std::experimental::apply(fun, std::forward<decltype(tuple)>(tuple));
        };
      }
    };

  } // end namespace detail


  template<typename... FromColumns, typename Fun, int Dim = 1>
  constexpr auto for_each(from_t<FromColumns...> f,
                          Fun fun,
                          dim_t<Dim> d = dim_t<1>{})
  {
    // wrap the function to be applied in the appropriate dimension
    auto fun_wrapper = detail::wrap_void_fun_for_dim<Dim>::impl(std::move(fun));

    return partial_for_each(f, std::move(fun_wrapper), [](auto& column){
      return std::ref(column.value());
    });
  }


} // end namespace cxtream
#endif
