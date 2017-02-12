/// \file
// Tuple utilities
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TUPLE_UTILS_HPP
#define TUPLE_UTILS_HPP


#include <type_traits>
#include <ostream>
#include <experimental/tuple>


namespace stream::utility {

  struct adl{};


  /* tuple_transform */


  template<typename Fun, typename... Ts>
  constexpr auto tuple_transform_impl(Fun& fun, Ts&&... args)
  {
    return std::make_tuple(fun(std::forward<Ts>(args))...);
  };


  template<typename Tuple, typename Fun>
  constexpr auto tuple_transform(Fun&& fun, Tuple&& tuple)
  {
    return std::experimental::apply(
      [fun=std::forward<Fun>(fun)](auto&&... args){
        return tuple_transform_impl(fun, std::forward<decltype(args)>(args)...); },
      std::forward<Tuple>(tuple));
  };


  /* tuple_contains */


  template<typename T, typename Tuple>
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
  };


} // end namespace stream::utility

#endif
