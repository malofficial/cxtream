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

  struct adl{};


  /* tuple_for_each */


  template<typename Fun, typename... Ts>
  constexpr Fun&& tuple_for_each_impl(Fun&& fun, Ts&&... args)
  {
    (..., fun(std::forward<Ts>(args)));
    return std::forward<Fun>(fun);
  }


  template<typename Tuple, typename Fun>
  constexpr auto tuple_for_each(Fun&& fun, Tuple&& tuple)
  {
    return std::experimental::apply(
      [fun=std::forward<Fun>(fun)](auto&&... args) mutable {
        return tuple_for_each_impl(std::forward<Fun>(fun),
                                   std::forward<decltype(args)>(args)...); },
      std::forward<Tuple>(tuple));
  }


  /* tuple_transform */


  template<typename Fun, typename... Ts>
  constexpr auto tuple_transform_impl(Fun&& fun, Ts&&... args)
  {
    return std::make_tuple(fun(std::forward<Ts>(args))...);
  }


  template<typename Tuple, typename Fun>
  constexpr auto tuple_transform(Fun&& fun, Tuple&& tuple)
  {
    return std::experimental::apply(
      [fun=std::forward<Fun>(fun)](auto&&... args) mutable {
        return tuple_transform_impl(fun, std::forward<decltype(args)>(args)...); },
      std::forward<Tuple>(tuple));
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


  template<typename Tuple, std::size_t N = std::tuple_size<std::decay_t<Tuple>>{}, std::size_t... Is>
  constexpr auto tuple_reverse_impl(Tuple&& tuple, std::index_sequence<Is...>)
  {
    return std::tuple<std::tuple_element_t<N - 1 - Is, std::decay_t<Tuple>>...>
      {std::get<N - 1 - Is>(std::forward<Tuple>(tuple))...};
  }

  template<typename Tuple>
  constexpr auto tuple_reverse(Tuple&& tuple)
  {
    return tuple_reverse_impl(std::forward<Tuple>(tuple),
                              std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
  }


  /* make_unique_tuple */


  template<typename Adl, typename Tuple>
  constexpr Tuple make_unique_tuple_impl(Adl _, Tuple&& tuple)
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


  template<typename... Args>
  constexpr auto make_unique_tuple(Args&&... args)
  {
    return make_unique_tuple_impl(adl{}, std::tuple<>{},
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
    return make_tuple_without_impl<Rem>(adl{}, std::tuple<>{},
                                        std::forward<Args>(args)...);
  }


  template<typename Rem, typename Tuple>
  constexpr auto tuple_remove(Tuple tuple)
  {
    return std::experimental::apply(
      [](auto&&... args){ return make_tuple_without<Rem>(std::forward<decltype(args)>(args)...); },
      std::move(tuple));
  }


  /* unzip */


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

  template<typename RangeT>
  auto unzip(RangeT range_of_tuples)
  {
    using tuple_type = ranges::range_value_t<RangeT>;
    constexpr auto tuple_size = std::tuple_size<tuple_type>{};
    constexpr auto indexes = std::make_index_sequence<tuple_size>{};
    auto range_size = ranges::size(range_of_tuples);

    auto tuple_of_ranges = vectorize_tuple<tuple_type>(range_size, indexes);

    for (std::size_t i = 0; i < range_size; ++i) {
      unzip_impl(tuple_of_ranges, i, std::move(range_of_tuples[i]), indexes);
    }

    return tuple_of_ranges;
  }


} // end namespace cxtream::utility

#endif
