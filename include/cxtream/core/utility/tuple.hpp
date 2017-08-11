/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_CORE_TUPLE_UTILS_HPP
#define CXTREAM_CORE_TUPLE_UTILS_HPP

#include <range/v3/core.hpp>
#include <range/v3/size.hpp>
#include <range/v3/to_container.hpp>

#include <experimental/tuple>
#include <ostream>
#include <type_traits>
#include <vector>

namespace cxtream::utility {

/// Get the first index of a type in a variadic template list
///
/// The first template argument is the argument to be searched.
/// The rest of the arguments is the variadic template.
///
/// Example:
/// \code
///     variadic_find<int, int, double, double>::value == 0
///     variadic_find<double, int, double, double>::value == 1
///     variadic_find<float, int, double, float>::value == 2
/// \endcode
template<typename T1, typename T2, typename... Ts>
struct variadic_find : std::integral_constant<std::size_t, variadic_find<T1, Ts...>{}+1> {
};

template<typename T, typename... Ts>
struct variadic_find<T, T, Ts...> : std::integral_constant<std::size_t, 0> {
};

// tuple_for_each //

namespace detail {

    struct tuple_for_each_adl {};

    template<typename Adl, typename Fun>
    constexpr Fun&& tuple_for_each_impl(Adl, Fun&& fun)
    {
        return std::forward<Fun>(fun);
    }

    template<typename Adl, typename Fun, typename Head, typename... Tail>
    constexpr Fun&& tuple_for_each_impl(Adl adl, Fun&& fun, Head&& head, Tail&&... tail)
    {
        fun(std::forward<Head>(head));
        return tuple_for_each_impl(adl, std::forward<Fun>(fun), std::forward<Tail>(tail)...);
    }

}  // namespace detail

/// Apply a function on each element of a tuple.
///
/// The order of application is from the first to the last element.
///
/// Example:
/// \code
///     auto tpl = std::make_tuple(5, 2.);
///     tuple_for_each([](auto& val) { std::cout << val << '\n'; }, tpl);
/// \endcode
///
/// \returns The function after application.
template<typename Tuple, typename Fun>
constexpr auto tuple_for_each(Fun&& fun, Tuple&& tuple)
{
    return std::experimental::apply(
      [fun = std::forward<Fun>(fun)](auto&&... args) mutable {
          return detail::tuple_for_each_impl(detail::tuple_for_each_adl{}, std::forward<Fun>(fun),
                                             std::forward<decltype(args)>(args)...);
      },
      std::forward<Tuple>(tuple));
}

// tuple_transform //

namespace detail {

    template<typename Fun, typename... Ts>
    constexpr auto tuple_transform_impl(Fun&& fun, Ts&&... args)
    {
        return std::make_tuple(fun(std::forward<Ts>(args))...);
    }

}  // end namespace detail

/// Transform each element of a tuple.
///
/// The order of application is unspecified.
///
/// Example:
/// \code
///    auto t1 = std::make_tuple(0, 10L, 5.);
///    auto t2 = tuple_transform([](const auto &v) { return v + 1; }, t1);
///    static_assert(std::is_same<std::tuple<int, long, double>, decltype(t2)>{});
///    assert(t2 == std::make_tuple(0 + 1, 10L + 1, 5. + 1));
/// \endcode
///
/// \returns The transformed tuple.
template<typename Tuple, typename Fun>
constexpr auto tuple_transform(Fun&& fun, Tuple&& tuple)
{
    return std::experimental::apply(
      [fun = std::forward<Fun>(fun)](auto&&... args) mutable {
          return detail::tuple_transform_impl(fun, std::forward<decltype(args)>(args)...);
      },
      std::forward<Tuple>(tuple));
}

/// Check whether a tuple contains a given type.
template<typename T, typename Tuple = void>
struct tuple_contains;

template<typename T, typename... Types>
struct tuple_contains<T, std::tuple<Types...>>
  : std::disjunction<std::is_same<std::decay_t<T>, std::decay_t<Types>>...> {
};

/// Makes a sub-tuple made of references to the original tuple (selected by type).
///
/// Example:
/// \code
///     auto tpl = std::make_tuple(0, 5., 'c');
///     auto subtpl = tuple_type_view<char, int>(t1);
///     static_assert(std::is_same<std::tuple<char&, int&>, decltype(subtpl)>{});
///     assert(subtpl == std::tuple<char, int>{'c', 0});
/// \endcode
///
/// \returns The view of the original tuple.
template<typename... Types, typename Tuple>
constexpr auto tuple_type_view(Tuple& tuple)
{
    return std::make_tuple(std::ref(std::get<Types>(tuple))...);
}

/// Makes a sub-tuple made of references to the original tuple (selected by index).
///
/// Example:
/// \code
///     auto tpl = std::make_tuple(0, 5., 'c');
///     auto subtpl = tuple_index_view<2, 0>(t1);
///     static_assert(std::is_same<std::tuple<char&, int&>, decltype(subtpl)>{});
///     assert(subtpl == std::tuple<char, int>{'c', 0});
/// \endcode
///
/// \returns The view of the original tuple.
template<std::size_t... Idxs, typename Tuple>
constexpr auto tuple_index_view(Tuple& tuple)
{
    return std::make_tuple(std::ref(std::get<Idxs>(tuple))...);
}

// tuple_reverse //

namespace detail {

    template<typename Tuple,
             std::size_t N = std::tuple_size<std::decay_t<Tuple>>{},
             std::size_t... Is>
    constexpr auto tuple_reverse_impl(Tuple&& tuple, std::index_sequence<Is...>)
    {
        return std::tuple<std::tuple_element_t<N - 1 - Is, std::decay_t<Tuple>>...>{
          std::get<N - 1 - Is>(std::forward<Tuple>(tuple))...};
    }

}  // namespace detail

/// Reverse a tuple.
///
/// Example:
/// \code
///     auto t1 = std::make_tuple(0, 5., 'c', 3, 'a');
///     auto t2 = tuple_reverse(t1);
///     assert(t2 == std::make_tuple('a', 3, 'c', 5., 0));
/// \endcode
template<typename Tuple>
constexpr auto tuple_reverse(Tuple&& tuple)
{
    return detail::tuple_reverse_impl(
      std::forward<Tuple>(tuple),
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
}

// make_unique_tuple //

namespace detail {

    struct make_unique_adl {};

    template<typename Adl, typename Tuple>
    constexpr Tuple make_unique_tuple_impl(Adl, Tuple&& tuple)
    {
        return std::forward<Tuple>(tuple);
    }

    template<typename Adl, typename Tuple, typename Head, typename... Tail,
             typename std::enable_if_t<!tuple_contains<Head, std::decay_t<Tuple>>{}, int> = 0>
    constexpr auto make_unique_tuple_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
        return make_unique_tuple_impl(
          adl, std::tuple_cat(std::forward<Tuple>(tuple),
                              std::tuple<Head>{std::forward<Head>(head)}),
          std::forward<Tail>(tail)...);
    }

    template<typename Adl, typename Tuple, typename Head, typename... Tail,
             typename std::enable_if_t<tuple_contains<Head, std::decay_t<Tuple>>{}, int> = 0>
    constexpr auto make_unique_tuple_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
        return make_unique_tuple_impl(adl, std::forward<Tuple>(tuple), std::forward<Tail>(tail)...);
    }

}  // namespace detail

/// Make tuple where no two elements have the same type.
///
/// Only the first element of each type is inserted into the tuple.
///
/// Example:
/// \code
///     auto tpl = make_unique_tuple(0, 1, '1', 'a', 2, 'b');
///     static_assert(std::is_same<std::tuple<int, char>, decltype(tpl)>{});
///     assert(tpl == std::make_tuple(0, '1'));
/// \endcode
template<typename... Args>
constexpr auto make_unique_tuple(Args&&... args)
{
    return detail::make_unique_tuple_impl(detail::make_unique_adl{}, std::tuple<>{},
                                          std::forward<Args>(args)...);
}

/// Concatenate two tuples and keep only the first element of each type.
///
/// Example:
/// \code
///     auto t1 = std::make_tuple(0, '1');
///     auto t2 = std::make_tuple(2, '3', 5.);
///     auto t3 = tuple_cat_unique(t1, t2);
///     static_assert(std::is_same<std::tuple<int, char, double>, decltype(t3)>{});
///     assert(t2 == std::make_tuple(0, '1', 5.));
/// \endcode
template <typename... Tuples>
constexpr auto tuple_cat_unique(Tuples&&... tuples)
{
    return std::experimental::apply(
      [](auto&&... args) { return make_unique_tuple(std::forward<decltype(args)>(args)...); },
      std::tuple_cat(std::forward<Tuples>(tuples)...));
}

/// Tuple pretty printing to std::ostream.
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

// tuple_remove //

namespace detail {

    struct make_tuple_without_adl {};

    template<typename Rem, typename Adl, typename Tuple>
    constexpr Tuple make_tuple_without_impl(Adl _, Tuple&& tuple)
    {
        return std::forward<Tuple>(tuple);
    }

    template<
      typename Rem, typename Adl, typename Tuple, typename Head, typename... Tail,
      typename std::enable_if_t<!std::is_same<std::decay_t<Head>, std::decay_t<Rem>>{}, int> = 0>
    constexpr auto make_tuple_without_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
        return make_tuple_without_impl<Rem>(
          adl, std::tuple_cat(std::forward<Tuple>(tuple),
                              std::tuple<Head>{std::forward<Head>(head)}),
          std::forward<Tail>(tail)...);
    }

    template<typename Rem, typename Adl, typename Tuple, typename Head, typename... Tail,
             typename std::enable_if_t<std::is_same<std::decay_t<Head>,
                                                    std::decay_t<Rem>>{}, int> = 0>
    constexpr auto make_tuple_without_impl(Adl adl, Tuple&& tuple, Head&& head, Tail&&... tail)
    {
        return make_tuple_without_impl<Rem>(adl, std::forward<Tuple>(tuple),
                                            std::forward<Tail>(tail)...);
    }

    template<typename Rem, typename... Args>
    constexpr auto make_tuple_without(Args&&... args)
    {
        return make_tuple_without_impl<Rem>(detail::make_tuple_without_adl{}, std::tuple<>{},
                                            std::forward<Args>(args)...);
    }

}  // namespace detail

/// Remove a type from a tuple.
///
/// Example:
/// \code
///     auto t1 = std::make_tuple(0, '1');
///     auto t2 = tuple_remove<int>(t1);
///     static_assert(std::is_same<std::tuple<char>, decltype(t2)>{});
///     assert(t2 == std::make_tuple('1'));
/// \endcode
template<typename Rem, typename Tuple>
constexpr auto tuple_remove(Tuple tuple)
{
    return std::experimental::apply(
      [](auto&&... args) {
          return detail::make_tuple_without<Rem>(std::forward<decltype(args)>(args)...);
      },
      std::move(tuple));
}

// unzip //

namespace detail {

    // wrap each type of a tuple in std::vector, i.e., make a tuple of empty vectors
    template<typename Tuple, std::size_t... Is>
    auto vectorize_tuple(std::index_sequence<Is...>)
    {
        return std::make_tuple(std::vector<std::tuple_element_t<Is, std::decay_t<Tuple>>>()...);
    }

    // push elements from the given tuple to the corresponding vectors in a tuple of vectors
    template<typename ToR, typename Tuple, std::size_t... Is>
    void push_unzipped(ToR& tuple_of_ranges, Tuple&& tuple, std::index_sequence<Is...>)
    {
        (..., (std::get<Is>(tuple_of_ranges).push_back(std::get<Is>(std::forward<Tuple>(tuple)))));
    }

    // if the size of the given range is known, return it, otherwise return 0
    template<typename Rng, CONCEPT_REQUIRES_(ranges::SizedRange<Rng>())>
    std::size_t safe_reserve_size(Rng&& rng)
    {
        return ranges::size(rng);
    }
    template<typename Rng, CONCEPT_REQUIRES_(!ranges::SizedRange<Rng>())>
    std::size_t safe_reserve_size(Rng&& rng)
    {
        return 0;
    }

    template<typename Rng>
    auto unzip_impl(Rng& range_of_tuples)
    {
        using tuple_type = ranges::range_value_type_t<Rng>;
        constexpr auto tuple_size = std::tuple_size<tuple_type>{};
        constexpr auto indices = std::make_index_sequence<tuple_size>{};
        std::size_t reserve_size = detail::safe_reserve_size(range_of_tuples);

        auto tuple_of_ranges = detail::vectorize_tuple<tuple_type>(indices);
        tuple_for_each([reserve_size](auto& rng) { rng.reserve(reserve_size); }, tuple_of_ranges);

        for (auto& v : range_of_tuples) {
            detail::push_unzipped(tuple_of_ranges, std::move(v), indices);
        }

        return tuple_of_ranges;
    }

}  // namespace detail

/// Unzips a range of tuples to a tuple of ranges.
///
/// Example:
/// \code
///     std::vector<std::tuple<int, double>> data{};
///     data.emplace_back(1, 5.);
///     data.emplace_back(2, 6.);
///     data.emplace_back(3, 7.);
///
///     std::vector<int> va;
///     std::vector<double> vb;
///     std::tie(va, vb) = unzip(data);
/// \endcode
template<typename Rng, CONCEPT_REQUIRES_(!ranges::View<Rng>())>
auto unzip(Rng range_of_tuples)
{
    // copy the given container and move elements out of it
    return detail::unzip_impl(range_of_tuples);
}

template<typename Rng, CONCEPT_REQUIRES_(ranges::View<Rng>())>
auto unzip(Rng&& view_of_tuples)
{
    return unzip(view_of_tuples | ranges::to_vector);
}

// maybe unzip //

namespace detail {

    template<bool Enable>
    struct unzip_if_impl
    {
        template<typename Rng>
        static decltype(auto) impl(Rng&& rng)
        {
            return unzip(std::forward<Rng>(rng));
        }
    };

    template<>
    struct unzip_if_impl<false>
    {
        template<typename Rng>
        static constexpr Rng&& impl(Rng&& rng)
        {
            return std::forward<Rng>(rng);
        }
    };

}  // namespace detail

/// Unzips a range of tuples to a tuple of ranges if a constexpr condition holds.
///
/// This method is enabled or disabled by its first template parameter.
/// If disabled, it returns identity. If enabled, it returns the same
/// thing as unzip() would return.
///
/// Example:
/// \code
///     std::vector<std::tuple<int, double>> data{};
///     data.emplace_back(1, 5.);
///     data.emplace_back(2, 6.);
///     data.emplace_back(3, 7.);
///
///     std::vector<int> va;
///     std::vector<double> vb;
///     std::tie(va, vb) = unzip_if<true>(data);
///
///     auto vc = unzip_if<false>(va);
/// \endcode
template<bool Enable, typename RangeT>
decltype(auto) unzip_if(RangeT&& range)
{
    return detail::unzip_if_impl<Enable>::impl(std::forward<RangeT>(range));
}

// range to tuple //

namespace detail {

    template<typename Rng, std::size_t... Is>
    constexpr auto range_to_tuple_impl(Rng rng, std::index_sequence<Is...>)
    {
        return std::make_tuple(std::move(ranges::at(std::forward<Rng>(rng), Is))...);
    }

}  // namespace detail

/// Converts a range to a tuple.
///
/// Example:
/// \code
///     std::vector<std::unique_ptr<int>> data;
///     data.emplace_back(std::make_unique<int>(5));
///     data.emplace_back(std::make_unique<int>(6));
///     data.emplace_back(std::make_unique<int>(7));
///
///     auto tpl = range_to_tuple<3>(std::move(data));
///     assert(tpl == std::make_tuple(5, 6, 7));
/// \endcode
template<std::size_t N, typename RARng>
constexpr auto range_to_tuple(RARng&& rng)
{
    assert(ranges::size(rng) >= N);
    return detail::range_to_tuple_impl(std::forward<RARng>(rng), std::make_index_sequence<N>{});
}

// times with index //

namespace detail {

    struct times_with_index_adl {};

    template<typename Adl, typename Fun, std::size_t I>
    constexpr auto times_with_index_impl(Adl, Fun&& fun, std::index_sequence<I>)
    {
        std::invoke(fun, std::integral_constant<std::size_t, I>{});
        return fun;
    }

    template<typename Adl, typename Fun,
             std::size_t IHead, std::size_t IHead2, std::size_t... ITail>
    constexpr auto times_with_index_impl(Adl adl, Fun&& fun,
                                         std::index_sequence<IHead, IHead2, ITail...>)
    {
        std::invoke(fun, std::integral_constant<std::size_t, IHead>{});
        return times_with_index_impl(adl, std::forward<Fun>(fun),
                                     std::index_sequence<IHead2, ITail...>{});
    }

}  // namespace detail

/// Repeat a function N times in compile time.
///
/// Example:
/// \code
///     auto tpl = std::make_tuple(1, 0.25, 'a');
///   
///     times_with_index<3>([&tpl](auto index) {
///         ++std::get<index>(tpl);
///     });
///     assert(tpl == std::make_tuple(2, 1.25, 'b'));
/// \endcode
template<std::size_t N, typename Fun>
constexpr auto times_with_index(Fun&& fun)
{
    return detail::times_with_index_impl(detail::times_with_index_adl{}, std::forward<Fun>(fun),
                                         std::make_index_sequence<N>{});
}

/// Similar to tuple_for_each(), but with index available.
///
/// Example:
/// \code
///     auto tpl = std::make_tuple(1, 2.);
///   
///     tuple_for_each_with_index([](auto& val, auto index) {
///         val += index;
///     }, tpl);
///   
///     assert(tpl == std::make_tuple(1, 3.));
/// \endcode
template <typename Fun, typename Tuple>
constexpr auto tuple_for_each_with_index(Fun&& fun, Tuple&& tuple)
{
    times_with_index<std::tuple_size<std::decay_t<Tuple>>{}>([&fun, &tuple](auto index) {
        std::invoke(fun, std::get<index>(tuple), index);
    });
}

// transform with index //

namespace detail {

    template <typename Fun, typename Tuple, std::size_t... Is>
    constexpr auto tuple_transform_with_index_impl(Fun&& fun, Tuple&& tuple,
                                                   std::index_sequence<Is...>)
    {
        return std::make_tuple(std::invoke(fun, std::get<Is>(std::forward<Tuple>(tuple)),
                                           std::integral_constant<std::size_t, Is>{})...);
    }

}  // namespace detail

/// Similar to tuple_transform(), but with index available.
///
/// Example:
/// \code
///     auto tpl = std::make_tuple(1, 0.25, 'a');
///   
///     auto tpl2 = tuple_transform_with_index([](auto&& elem, auto index) {
///         return elem + index;
///     }, tpl);
///   
///     assert(tpl2 == std::make_tuple(1, 1.25, 'c'));
/// \endcode
template <typename Fun, typename Tuple>
constexpr auto tuple_transform_with_index(Fun&& fun, Tuple&& tuple)
{
    return detail::tuple_transform_with_index_impl(
      std::forward<Fun>(fun), std::forward<Tuple>(tuple),
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
}

}  // namespace cxtream::utility

#endif
