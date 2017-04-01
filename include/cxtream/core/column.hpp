/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_COLUMN_HPP
#define CXTREAM_CORE_COLUMN_HPP

#include <type_traits>
#include <vector>

namespace cxtream {

/// Base class for cxtream columns.
///
/// Stores a vector of given types and provides convenient constructors.
template <typename T, bool = std::is_copy_constructible<T>{}>
class column_base {
private:
    std::vector<T> value_;

public:

    // constructors //

    column_base() = default;
    column_base(T&& rhs) { value_.emplace_back(std::move(rhs)); }
    column_base(const T& rhs)
      : value_{rhs}
    {}

    column_base(std::vector<T>&& rhs)
      : value_{std::move(rhs)}
    {}

    column_base(const std::vector<T>& rhs)
      : value_{rhs}
    {}

    // value accessors //

    std::vector<T>& value() { return value_; }
    const std::vector<T>& value() const { return value_; }
};

/// Specialization of column_base for non-copy-constructible types.
template <typename T>
struct column_base<T, false> : column_base<T, true> {
    using column_base<T, true>::column_base;

    column_base() = default;
    column_base(const T& rhs) = delete;
    column_base(const std::vector<T>& rhs) = delete;
};

}  // namespace cxtream

/// Macro for fast column definition.
///
/// Basically it creates a new type derived from column_base.
#define CXTREAM_DEFINE_COLUMN(col_name, col_type)             \
struct col_name : cxtream::column_base<col_type> {            \
    using cxtream::column_base<col_type>::column_base;        \
    static constexpr const char* name() { return #col_name; } \
};

#endif
