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

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>
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

/// Add a number to all values in std::index_sequence.
///
/// Example:
/// \code
///     std::is_same<decltype(plus<2>(std::index_sequence<1, 3, 4>{})),
///                                   std::index_sequence<3, 5, 6>>;
/// \endcode
template<std::size_t Value, std::size_t... Is>
constexpr std::index_sequence<(Value + Is)...> plus(std::index_sequence<Is...>)
{
    return {};
}

/// Make std::index_sequence with the given offset.
///
/// Example:
/// \code
///     std::is_same<decltype(make_offset_index_sequence<3, 4>()),
///                           std::index_sequence<3, 4, 5, 6>>;
/// \endcode
template <std::size_t Offset, std::size_t N>
using make_offset_index_sequence = decltype(plus<Offset>(std::make_index_sequence<N>{}));

// tuple_for_each //

namespace detail {

    template<typename Fun, typename Tuple, std::size_t... Is>
    constexpr Fun tuple_for_each_impl(Tuple&& tuple, Fun&& fun, std::index_sequence<Is...>)
    {
        (..., (std::invoke(fun, std::get<Is>(std::forward<Tuple>(tuple)))));
        return fun;
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
    constexpr std::size_t tuple_size = std::tuple_size<std::decay_t<Tuple>>{};
    return detail::tuple_for_each_impl(std::forward<Tuple>(tuple),
                                       std::forward<Fun>(fun),
                                       std::make_index_sequence<tuple_size>{});
}

// tuple_transform //

namespace detail {

    template<typename Fun, typename Tuple, std::size_t... Is>
    constexpr auto tuple_transform_impl(Tuple&& tuple, Fun&& fun, std::index_sequence<Is...>)
    {
        return std::make_tuple(std::invoke(fun, std::get<Is>(std::forward<Tuple>(tuple)))...);
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
    constexpr std::size_t tuple_size = std::tuple_size<std::decay_t<Tuple>>{};
    return detail::tuple_transform_impl(std::forward<Tuple>(tuple),
                                        std::forward<Fun>(fun),
                                        std::make_index_sequence<tuple_size>{});
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

/// Concatenate two tuples and keep only the first element of each type.
///
/// Note: All the reference types are decayed during this operation.
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
    auto values = std::tuple_cat(std::forward<Tuples>(tuples)...);
    auto type_value_pairs = boost::hana::transform(std::move(values), [](auto v) {
        return boost::hana::make_pair(boost::hana::type_c<decltype(v)>, std::move(v));
    });
    auto map = boost::hana::to_map(std::move(type_value_pairs));
    return boost::hana::to<boost::hana::ext::std::tuple_tag>(boost::hana::values(std::move(map)));
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
    return utility::tuple_print(out, tuple, std::make_index_sequence<sizeof...(Ts)>{});
}

// tuple_remove //

/// Remove a type from a tuple.
///
/// Note: All the reference types are decayed during this operation.
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
    return boost::hana::remove_if(std::move(tuple), [](const auto& a) {
        return std::is_same<std::decay_t<decltype(a)>, std::decay_t<Rem>>{};
    });
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
        utility::tuple_for_each([reserve_size](auto& rng) { rng.reserve(reserve_size); },
                                tuple_of_ranges);

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
template<typename Rng, CONCEPT_REQUIRES_(ranges::Range<Rng>() && !ranges::View<Rng>())>
auto unzip(Rng range_of_tuples)
{
    // copy the given container and move elements out of it
    return detail::unzip_impl(range_of_tuples);
}

/// Specialization of unzip function for views.
template<typename Rng, CONCEPT_REQUIRES_(ranges::View<Rng>())>
auto unzip(Rng view_of_tuples)
{
    return utility::unzip(view_of_tuples | ranges::to_vector);
}

// maybe unzip //

namespace detail {

    template<bool Enable>
    struct unzip_if_impl
    {
        template<typename Rng>
        static decltype(auto) impl(Rng&& rng)
        {
            return utility::unzip(std::forward<Rng>(rng));
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

    template<typename Fun, std::size_t... Is>
    constexpr Fun times_with_index_impl(Fun&& fun, std::index_sequence<Is...>)
    {
        (..., (std::invoke(fun, std::integral_constant<std::size_t, Is>{})));
        return fun;
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
constexpr Fun times_with_index(Fun&& fun)
{
    return detail::times_with_index_impl(std::forward<Fun>(fun), std::make_index_sequence<N>{});
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
    return utility::times_with_index<std::tuple_size<std::decay_t<Tuple>>{}>(
      [&fun, &tuple](auto index) {
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
