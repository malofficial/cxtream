/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_PYTHON_UTILITY_PYBOOST_COLUMN_CONVERTER_HPP
#define CXTREAM_PYTHON_UTILITY_PYBOOST_COLUMN_CONVERTER_HPP

#include <cxtream/core/utility/tuple.hpp>

#include <boost/python.hpp>

#include <string>
#include <vector>

namespace cxtream::python::utility {

// recursive transformation from a multidimensional vector to a python list //

namespace detail {

    template <typename T>
    struct to_list_impl {
        template <typename U>
        static U&& impl(U&& val)
        {
            return std::forward<U>(val);
        }
    };

    template <typename T>
    struct to_list_impl<std::vector<T>> {
        static boost::python::list impl(std::vector<T> vec)
        {
            boost::python::list res;
            for (auto& val : vec) res.append(to_list_impl<T>::impl(std::move(val)));
            return res;
        }
    };

}  // namespace detail

/// Create a python list out of std::vector.
///
/// If the vector is multidimensional, i.e., std::vector<std::vector<...>>,
/// the conversion is applied recursively.
template <typename Vector>
boost::python::list to_list(Vector&& v)
{
    return detail::to_list_impl<std::decay_t<Vector>>::impl(std::forward<Vector>(v));
}

/// Convert a tuple of cxtream columns into python dict.
///
/// The dict is indexed by column.name and the value is column.value.
/// Batches are converted to python lists. If the batch value is a multidimensional
/// std::vector<std::vector<...>>, it is converted to multidimensional python list.
template <typename Tuple>
boost::python::dict columns_to_dict(Tuple tuple)
{
    boost::python::dict res;
    cxtream::utility::tuple_for_each([&res](auto& column) {
        res[column.name()] = to_list(std::move(column.value()));
    }, tuple);
    return res;
}

}  // namespace cxtream::python::utility
#endif
