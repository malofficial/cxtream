/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#include <cxtream/core/stream/column.hpp>
#include <cxtream/python/pyboost_initialize.hpp>
#include <cxtream/python/utility/pyboost_column_converter.hpp>

#include <tuple>
#include <vector>

namespace py = boost::python;

std::vector<int> vec1d = {1, 2, 3};
std::vector<std::vector<int>> vec2d = {vec1d, vec1d, vec1d};
std::vector<std::vector<std::vector<int>>> vec3d = {vec2d, vec2d, vec2d};

// test to_list //

py::list list1d()
{
    return cxtream::python::utility::to_list(vec1d);
}

py::list list2d()
{
    return cxtream::python::utility::to_list(vec2d);
}

py::list list3d()
{
    return cxtream::python::utility::to_list(vec3d);
}

// test columns_to_dict //

CXTREAM_DEFINE_COLUMN(Int, int)
CXTREAM_DEFINE_COLUMN(Double, double)

py::dict columns()
{
    using cxtream::python::utility::columns_to_dict;
    return columns_to_dict(std::tuple<Int, Double>{{1, 2}, {9., 10.}});
}

BOOST_PYTHON_MODULE(pyboost_column_converter_py_cpp)
{
    // initialize cxtream OpenCV converters, exceptions, etc.
    cxtream::python::initialize();

    // expose the functions
    py::def("list1d", list1d);
    py::def("list2d", list2d);
    py::def("list3d", list3d);
    py::def("columns", columns);
}
