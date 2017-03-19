/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_TUPLE_UTILS_HPP
#define CXTREAM_CORE_TUPLE_UTILS_HPP

#include <type_traits>
#include <ostream>
#include <experimental/tuple>
#include <range/v3/size.hpp>
#include <range/v3/core.hpp>

namespace cxtream::utility {


  /* tuple_for_each */


  namespace detail {

    struct tuple_for_each_adl{};

    template<typename Adl, typename Fun>
    constexpr Fun&& tuple_for_each_impl(Adl, Fun&& fun)
    {
      return std::forward<Fun>(fun);
    }

    template<typename Adl, typename Fun, typename Head, typename... Tail>
    constexpr Fun&& tuple_for_each_impl(Adl adl, Fun&& fun, Head&& head, Tail&&... tail)
    {
      fun(std::forward<Head>(head));
      return tuple_for_each_impl(adl,
                                 std::forward<Fun>(fun),
                                 std::forward<Tail>(tail)...);
    }

  }


  template<typename Tuple, typename Fun>
  constexpr auto tuple_for_each(Fun&& fun, Tuple&& tuple)
  {
    return std::experimental::apply(
      [fun=std::forward<Fun>(fun)](auto&&... args) mutable {
        return detail::tuple_for_each_impl(detail::tuple_for_each_adl{},
                                           std::forward<Fun>(fun),
                                           std::forward<decltype(args)>(args)...); },
      std::forward<Tuple>(tuple));
  }


  /* tuple_transform */

  // beware, the order of application is unspecified

  namespace detail {

    template<typename Fun, typename... Ts>
    constexpr auto tuple_transform_impl(Fun&& fun, Ts&&... args)
    {
      return std::make_tuple(fun(std::forward<Ts>(args))...);
    }

  } // end namespace detail


  template<typename Tuple, typename Fun>
  constexpr auto tuple_transform(Fun&& fun, Tuple&& tuple)
  {
    return std::experimental::apply(
      [fun=std::forward<Fun>(fun)](auto&&... args) mutable {
        return detail::tuple_transform_impl(
          fun, std::forward<decltype(args)>(args)...);
      }, std::forward<Tuple>(tuple));
  }


  /* tuple_contains */


  template<typename T, typename Tuple = void>
  struct tuple_contains;

  template<typename T, typename... Types>
  struct tuple_contains<T, std::tuple<Types...>>
    : std::disjunction<std::is_same<std::decay_t<T>, std::decay_t<Types>>...> {};


  /* tuple_type_view */


  template<typename... Types, typename Tuple>
  constexpr auto tuple_type_view(Tuple& tuple)
  {
    return std::make_tuple(std::ref(std::get<Types>(tuple))...);
  }


  /* tuple_reverse */


  namespace detail {

    template<typename Tuple, std::size_t N = std::tuple_size<std::decay_t<Tuple>>{}, std::size_t... Is>
    constexpr auto tuple_reverse_impl(Tuple&& tuple, std::index_sequence<Is...>)
    {
      return std::tuple<std::tuple_element_t<N - 1 - Is, std::decay_t<Tuple>>...>
        {std::get<N - 1 - Is>(std::forward<Tuple>(tuple))...};
    }

  } // end namespace detail

  template<typename Tuple>
  constexpr auto tuple_reverse(Tuple&& tuple)
  {
    return detail::tuple_reverse_impl(
      std::forward<Tuple>(tuple),
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
  }


  /* make_unique_tuple */


  namespace detail {

    struct make_unique_adl{};

    template<typename Adl, typename Tuple>
    constexpr Tuple make_unique_tuple_impl(Adl, Tuple&& tuple)
    {
      return std::forward<Tuple>(tuple);
    }

    template<typename Adl, typename Tuple, typename Head, typename... Tail,
             typename std::enable_if_t<!tuple_contains<Head, std::decay_t<Tuple>>{}, int> = 0>
    constexpr auto make_unique_tuple_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
      return make_unique_tuple_impl(adl, std::tuple_cat(std::forward<Tuple>(tuple),
                                                        std::tuple<Head>{std::forward<Head>(head)}),
                                    std::forward<Tail>(tail)...);
    }

    template<typename Adl, typename Tuple, typename Head, typename... Tail,
             typename std::enable_if_t<tuple_contains<Head, std::decay_t<Tuple>>{}, int> = 0>
    constexpr auto make_unique_tuple_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
      return make_unique_tuple_impl(adl, std::forward<Tuple>(tuple),
                                    std::forward<Tail>(tail)...);
    }

  } // end namespace detail


  template<typename... Args>
  constexpr auto make_unique_tuple(Args&&... args)
  {
    return detail::make_unique_tuple_impl(
      detail::make_unique_adl{}, std::tuple<>{},
      std::forward<Args>(args)...);
  }


  /* tuple_cat_unique */


  template<typename... Tuples>
  constexpr auto tuple_cat_unique(Tuples&&... tuples)
  {
    return std::experimental::apply(
      [](auto&&... args){ return make_unique_tuple(std::forward<decltype(args)>(args)...); },
      std::tuple_cat(std::forward<Tuples>(tuples)...));
  }


  /* tuple_print */


  template<typename Tuple, size_t... Is>
  std::ostream& tuple_print(std::ostream& out, const Tuple& tuple, std::index_sequence<Is...>)
  {
    out << "(";
    (..., (out << (Is == 0 ? "" : ", ") << std::get<Is>(tuple)));
    out << ")";
    return out;
  }

  template<typename... Ts>
  std::ostream& operator<<(std::ostream& out, const std::tuple<Ts...>& tuple)
  {
    return tuple_print(out, tuple, std::make_index_sequence<sizeof...(Ts)>{});
  }


  /* tuple_remove */


  namespace detail {

    struct make_tuple_without_adl{};

    template<typename Rem, typename Adl, typename Tuple>
    constexpr Tuple make_tuple_without_impl(Adl _, Tuple&& tuple)
    {
      return std::forward<Tuple>(tuple);
    }

    template<typename Rem, typename Adl, typename Tuple, typename Head, typename... Tail,
             typename std::enable_if_t<!std::is_same<std::decay_t<Head>, std::decay_t<Rem>>{}, int> = 0>
    constexpr auto make_tuple_without_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
      return make_tuple_without_impl<Rem>(adl, std::tuple_cat(std::forward<Tuple>(tuple),
                                                              std::tuple<Head>{std::forward<Head>(head)}),
                                          std::forward<Tail>(tail)...);
    }

    template<typename Rem, typename Adl, typename Tuple, typename Head, typename... Tail,
             typename std::enable_if_t<std::is_same<std::decay_t<Head>, std::decay_t<Rem>>{}, int> = 0>
    constexpr auto make_tuple_without_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
      return make_tuple_without_impl<Rem>(adl, std::forward<Tuple>(tuple),
                                          std::forward<Tail>(tail)...);
    }


    template<typename Rem, typename... Args>
    constexpr auto make_tuple_without(Args&&... args)
    {
      return make_tuple_without_impl<Rem>(
        detail::make_tuple_without_adl{}, std::tuple<>{},
        std::forward<Args>(args)...);
    }

  } // end namespace detail


  template<typename Rem, typename Tuple>
  constexpr auto tuple_remove(Tuple tuple)
  {
    return std::experimental::apply(
      [](auto&&... args){
        return detail::make_tuple_without<Rem>(std::forward<decltype(args)>(args)...);
      },
      std::move(tuple));
  }


  /* unzip */


  namespace detail {

    // wrap each type of Tuple in std::vector
    template<typename Tuple, std::size_t... Is>
    auto vectorize_tuple(std::size_t size, std::index_sequence<Is...>)
    {
      return std::make_tuple(
        std::vector<std::tuple_element_t<Is, std::decay_t<Tuple>>>(size)...);
    }

    template<typename ToR, typename Tuple, std::size_t... Is>
    void unzip_impl(ToR& tuple_of_ranges, std::size_t i,
                    Tuple&& tuple, std::index_sequence<Is...>)
    {
      (..., (std::get<Is>(tuple_of_ranges)[i] = std::get<Is>(std::forward<Tuple>(tuple))));
    }

  } // end namespace detail

  template<typename RangeT>
  auto unzip(RangeT range_of_tuples)
  {
    using tuple_type = ranges::range_value_t<RangeT>;
    constexpr auto tuple_size = std::tuple_size<tuple_type>{};
    constexpr auto indexes = std::make_index_sequence<tuple_size>{};
    auto range_size = ranges::size(range_of_tuples);

    auto tuple_of_ranges = detail::vectorize_tuple<tuple_type>(range_size, indexes);
    tuple_for_each([range_size](auto& rng){ rng.reserve(range_size); }, tuple_of_ranges);

    for (std::size_t i = 0; i < range_size; ++i) {
      detail::unzip_impl(tuple_of_ranges, i, std::move(range_of_tuples[i]), indexes);
    }

    return tuple_of_ranges;
  }


  /* range to tuple */


  namespace detail {

    template<typename Rng, std::size_t... Is>
    constexpr auto range_to_tuple_impl(Rng rng, std::index_sequence<Is...>)
    {
      return std::make_tuple(std::move(ranges::at(std::forward<Rng>(rng), Is))...);
    }

  }

  template<std::size_t N, typename RARng>
  constexpr auto range_to_tuple(RARng&& rng)
  {
    assert(ranges::size(rng) >= N);
    return detail::range_to_tuple_impl(
      std::forward<RARng>(rng),
      std::make_index_sequence<N>{}
    );
  }


  /* times with index */


  namespace detail {

    struct times_with_index_adl{};

    template<typename Adl, typename Fun, std::size_t I>
    constexpr auto times_with_index_impl(
      Adl,
      Fun&& fun,
      std::index_sequence<I>)
    {
      std::invoke(fun, std::integral_constant<std::size_t, I>{});
      return fun;
    }

    template<
      typename Adl, typename Fun,
      std::size_t IHead, std::size_t IHead2, std::size_t... ITail>
    constexpr auto times_with_index_impl(
      Adl adl,
      Fun&& fun,
      std::index_sequence<IHead, IHead2, ITail...>)
    {
      std::invoke(fun, std::integral_constant<std::size_t, IHead>{});
      return times_with_index_impl(
        adl,
        std::forward<Fun>(fun),
        std::index_sequence<IHead2, ITail...>{}
      );
    }

  } // end namespace detail


  template<std::size_t N, typename Fun>
  constexpr auto times_with_index(Fun&& fun)
  {
    return detail::times_with_index_impl(
      detail::times_with_index_adl{},
      std::forward<Fun>(fun),
      std::make_index_sequence<N>{}
    );
  }


  /* for_each with index */


  template<typename Fun, typename Tuple>
  constexpr auto tuple_for_each_with_index(Fun&& fun, Tuple&& tuple)
  {
    times_with_index<std::tuple_size<std::decay_t<Tuple>>{}>(
      [&fun, &tuple](auto index){
        std::invoke(fun, std::get<index>(tuple), index);
      }
    );
  }


  /* transform with index */

  // beware, the order of application is unspecified

  namespace detail {

    template<typename Fun, typename Tuple, std::size_t... Is>
    constexpr auto tuple_transform_with_index_impl(
      Fun&& fun,
      Tuple&& tuple,
      std::index_sequence<Is...>)
    {
      return std::make_tuple(
        std::invoke(
          fun,
          std::get<Is>(std::forward<Tuple>(tuple)),
          std::integral_constant<std::size_t, Is>{}
        )...
      );
    }

  } // end namespace detail


  template<typename Fun, typename Tuple>
  constexpr auto tuple_transform_with_index(Fun&& fun, Tuple&& tuple)
  {
    return detail::tuple_transform_with_index_impl(
      std::forward<Fun>(fun),
      std::forward<Tuple>(tuple),
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{}
    );
  }

} // end namespace cxtream::utility

#endif
