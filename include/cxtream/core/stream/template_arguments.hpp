/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_CORE_STREAM_TEMPLATE_ARGUMENTS_HPP
#define CXTREAM_CORE_STREAM_TEMPLATE_ARGUMENTS_HPP

#include <functional>

namespace cxtream::stream {

/// Helper type representing columns which should be transformed.
template <typename... Columns>
struct from_t {
};

template <typename... Columns>
auto from = from_t<Columns...>{};

/// Helper type representing columns to which should a transformation save the result.
template <typename... Columns>
struct to_t {
};

template <typename... Columns>
auto to = to_t<Columns...>{};

/// Helper type representing columns by which one should filter etc.
template <typename... Columns>
struct by_t {
};

template <typename... Columns>
auto by = by_t<Columns...>{};

/// Helper type representing dimension.
template <int Dim>
struct dim_t {
};

template <int Dim>
auto dim = dim_t<Dim>{};

/// Function object type forwarding the given object back to the caller.
struct identity_t {
    template <typename T>
    constexpr T&& operator()(T&& val) const noexcept
    {
        return std::forward<T>(val);
    }
};

auto identity = identity_t{};

/// Function object type wrapping the given object in std::reference_wrapper.
struct ref_wrap_t {
    template <typename T>
    constexpr decltype(auto) operator()(T& val) const noexcept
    {
        return std::ref(val);
    }
};

auto ref_wrap = ref_wrap_t{};

}  // namespace cxtream::stream
#endif
