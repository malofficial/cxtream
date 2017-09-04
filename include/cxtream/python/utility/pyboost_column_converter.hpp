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
#include <cxtream/python/range.hpp>
#include <cxtream/python/utility/pyboost_ndarray_converter.hpp>

#include <boost/python.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace cxtream::python::utility {

// recursive transformation from a multidimensional vector to a python iterator //

namespace detail {

    // conversion of std::vector to a python list-like type //

    template<typename T>
    struct vector_to_python_impl {
        template<typename U>
        static U&& impl(U&)
        {
            static_assert("Only std::vectors are supported in vector_to_python function.");
        }
    };

    template<typename T>
    struct vector_to_python_impl<std::vector<T>> {
        static PyObject* impl(std::vector<T>& vec)
        {
            return utility::to_ndarray(vec);
        }
    };

    template<typename T>
    struct vector_to_python_impl<std::vector<std::vector<T>>> {
        static PyObject* impl(std::vector<std::vector<T>>& vec)
        {
            PyObject* list{PyList_New(vec.size())};
            if (!list) throw std::runtime_error{"Unable to create Python list."};
            for (std::size_t i = 0; i < vec.size(); ++i) {
                PyList_SET_ITEM(list, i, vector_to_python_impl<std::vector<T>>::impl(vec[i]));
            }
            return list;
        }
    };

}  // namespace detail

/// Create a python list-like object out of a multidimensional std::vector.
///
/// If the vector is multidimensional, i.e., std::vector<std::vector<...>>,
/// the resulting structure will be multidimensional as well.
template<typename T>
boost::python::object to_python(std::vector<T> v)
{
    namespace py = boost::python;
    py::handle<> py_obj_handle{detail::vector_to_python_impl<std::vector<T>>::impl(v)};
    return py::object{py_obj_handle};
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
