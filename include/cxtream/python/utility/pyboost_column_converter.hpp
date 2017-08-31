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
#include <cxtream/python/pyboost_range.hpp>

#include <boost/python.hpp>

#include <string>
#include <vector>

namespace cxtream::python::utility {

// recursive transformation from a multidimensional vector to a python iterator //

namespace detail {

    // conversion of std::vector to a python list-like type //

    template<typename T>
    struct to_python_impl {
        template<typename U>
        static void impl(U)
        {
            static_assert("Only std::vector is supported in to_python function.");
        }
    };

    template<typename T>
    struct to_python_impl<std::vector<T>> {
        static auto impl(std::vector<T>& vec)
        {
            return range<std::vector<T>>{std::move(vec)};
        }
    };

    template<typename T>
    struct to_python_impl<std::vector<std::vector<T>>> {
        static auto impl(std::vector<std::vector<T>>& vec)
        {
            using inner_type = decltype(to_python_impl<std::vector<T>>::impl(vec[0]));
            std::vector<inner_type> py_data;
            py_data.reserve(vec.size());
            for (auto& val : vec) py_data.push_back(to_python_impl<std::vector<T>>::impl(val));
            return range<std::vector<inner_type>>{std::move(py_data)};
        }
    };

}  // namespace detail

/// Create a python list-like object out of a multidimensional std::vector.
///
/// If the vector is multidimensional, i.e., std::vector<std::vector<...>>,
/// the resulting structure will be multidimensional as well.
template<typename Vector>
auto to_python(Vector v)
{
    return detail::to_python_impl<Vector>::impl(v);
}

/// Convert a tuple of cxtream columns into a python dict.
///
/// The dict is indexed by column.name and the value is column.value.
/// Batches are converted to python iterators. If the batch value is a multidimensional
/// std::vector<std::vector<...>>, it is converted to multidimensional python iterator.
template<typename Tuple>
boost::python::dict columns_to_python(Tuple tuple)
{
    boost::python::dict res;
    cxtream::utility::tuple_for_each([&res](auto& column) {
        res[column.name()] = to_python(std::move(column.value()));
    }, tuple);
    return res;
}

}  // namespace cxtream::python::utility
#endif
