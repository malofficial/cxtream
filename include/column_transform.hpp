/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_COLUMN_TRANSFORM_HPP
#define CXTREAM_COLUMN_TRANSFORM_HPP

#include <type_traits>
#include <utility>
#include <vector>

#include <range/v3/view/for_each.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <utility/tuple.hpp>
#include <view/buffer.hpp>

namespace cxtream {

  using namespace ranges;


  /* column definition macro */


  template<typename T, bool = std::is_copy_constructible<T>{}>
  struct column_base
  {
    std::vector<T> value;

    column_base() = default;
    column_base(T&& rhs)
    {
      value.emplace_back(std::move(rhs));
    }
    column_base(const T& rhs)
      : value{rhs}
    { }
    column_base(std::vector<T>&& rhs)
      : value{std::move(rhs)}
    { }
    column_base(const std::vector<T>& rhs)
      : value{rhs}
    { }
  };

  template<typename T>
  struct column_base<T, false> : column_base<T, true>
  {
    using column_base<T, true>::column_base;

    column_base() = default;
    column_base(const T& rhs) = delete;
    column_base(const std::vector<T>& rhs) = delete;
  };

#define CXTREAM_DEFINE_COLUMN(col_name, col_type) \
struct col_name : cxtream::column_base<col_type> { \
  using cxtream::column_base<col_type>::column_base; \
  static constexpr const char* name = #col_name; \
};


  /* helper types */


  template<typename... Columns>
  struct from_t {};

  template<typename... Columns>
  auto from = from_t<Columns...>{};

  template<typename... Columns>
  struct to_t {};

  template<typename... Columns>
  auto to = to_t<Columns...>{};

  template<int Dim>
  struct dim_t {};

  template<int Dim>
  auto dim = dim_t<Dim>{};


  /* helper projections */


  struct identity_t
  {
    template<typename T>
    constexpr T&& operator()(T&& val) const noexcept
    {
      return std::forward<T>(val);
    }
  };

  auto identity = identity_t{};

  struct ref_wrap_t
  {
    template<typename T>
    constexpr decltype(auto) operator()(T& val) const noexcept
    {
      return std::ref(val);
    }
  };

  auto ref_wrap = ref_wrap_t{};


  /* partial_transform */


  template<typename... FromTypes, typename... ToTypes,
           typename Fun, typename Projection = ref_wrap_t>
  constexpr auto partial_transform(from_t<FromTypes...>, to_t<ToTypes...>,
                                   Fun fun, Projection proj = Projection{})
  {
    static_assert(sizeof...(ToTypes) > 0, "For non-transforming operations, please"
        " use the partial_for_each.");

    return view::transform([fun=std::move(fun), proj=std::move(proj)](auto&& source){
      // build the view for the transformer, i.e., slice and project
      const auto slice_view =
        utility::tuple_transform(proj, utility::tuple_type_view<FromTypes...>(source));
      // process the transformer's result and convert it to the requested types
      std::tuple<ToTypes...> result{std::invoke(fun, slice_view)};
      // replace the corresponding fields
      return utility::tuple_cat_unique(std::move(result), std::forward<decltype(source)>(source));
    });
  }


  /* column_transform */


  // apply fun to some dimension
  template<int Dim>
  struct wrap_fun_for_dim
  {
    template<typename Fun>
    static constexpr auto impl(Fun fun)
    {
      return [fun=std::move(fun)](auto&& tuple_of_ranges){
        auto range_of_tuples =
            std::experimental::apply(
                view::zip,
                std::forward<decltype(tuple_of_ranges)>(tuple_of_ranges))
          | view::transform(wrap_fun_for_dim<Dim - 1>::impl(fun));
        return utility::unzip(std::move(range_of_tuples));
      };
    }
  };

  // apply fun to the entire batch
  template<>
  struct wrap_fun_for_dim<0>
  {
    template<typename Fun>
    static constexpr auto impl(Fun fun)
    {
      return [fun=std::move(fun)](auto&& tuple){
        return std::experimental::apply(fun, std::forward<decltype(tuple)>(tuple));
      };
    }
  };

  template<typename... FromColumns, typename... ToColumns, typename Fun, int Dim = 1>
  constexpr auto column_transform(from_t<FromColumns...> f,
                                  to_t<ToColumns...> t,
                                  Fun fun,
                                  dim_t<Dim> d = dim_t<1>{})
  {
    // wrap the function to be applied in the appropriate dimension
    auto fun_wrapper = wrap_fun_for_dim<Dim>::impl(std::move(fun));

    return partial_transform(f, t, std::move(fun_wrapper), [](auto& column){
      return std::ref(column.value);
    });
  }


  /* partial_for_each */


  template<typename... FromTypes, typename Fun, typename Projection = ref_wrap_t>
  constexpr auto partial_for_each(from_t<FromTypes...>,
                                  Fun fun, Projection proj = Projection{})
  {
    return view::transform([fun=std::move(fun), proj=std::move(proj)](auto&& source) mutable {
      // build the view for the transformer, i.e., slice and project
      const auto slice_view =
        utility::tuple_transform(proj, utility::tuple_type_view<FromTypes...>(source));
      // apply the function
      std::invoke(fun, slice_view);
      // return the original
      return std::forward<decltype(source)>(source);
    });
  }


  /* column_for_each */


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
                view::zip,
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


  template<typename... FromColumns, typename Fun, int Dim = 1>
  constexpr auto column_for_each(from_t<FromColumns...> f,
                                 Fun fun,
                                 dim_t<Dim> d = dim_t<1>{})
  {
    // wrap the function to be applied in the appropriate dimension
    auto fun_wrapper = wrap_void_fun_for_dim<Dim>::impl(std::move(fun));

    return partial_for_each(f, std::move(fun_wrapper), [](auto& column){
      return std::ref(column.value);
    });
  }


  /* column_drop */


  template<typename Column>
  constexpr auto column_drop_fn()
  {
    return view::transform([](auto&& source) {
      return utility::tuple_remove<Column>(std::forward<decltype(source)>(source));
    });
  }

  // allow calls without parentheses
  template<typename Column>
  auto column_drop = column_drop_fn<Column>();


  /* column_create */


  template<typename Column>
  constexpr auto column_create_fn()
  {
    return view::transform([](auto&& source){
        return std::tuple<Column>{std::forward<decltype(source)>(source)};
    });
  }

  // allow calls without parentheses
  template<typename Column>
  auto column_create = column_create_fn<Column>();


} // end namespace cxtream
#endif
