/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#include <cxtream/python/initialize.hpp>
#include <cxtream/python/utility/pyboost_ndarray_converter.hpp>

#include <vector>

namespace py = boost::python;

struct IntWrap {
    int num;
};

std::vector<bool>    empty_vec  = {};
std::vector<char>    char_vec   = {'o', 'o', 'p', 's', '!'};
std::vector<int>     int_vec    = {-1, 0, 1, 2, 3};
std::vector<double>  double_vec = {-1.5, -0.5, 0.5, 1.5, 2.5};
std::vector<IntWrap> object_vec = {IntWrap{1}, IntWrap{2}, IntWrap{3}};

auto py_empty_vec()
{
    return cxtream::python::utility::to_ndarray(empty_vec);
}

auto py_char_vec()
{
    return cxtream::python::utility::to_ndarray(char_vec);
}

auto py_int_vec()
{
    return cxtream::python::utility::to_ndarray(int_vec);
}

auto py_double_vec()
{
    return cxtream::python::utility::to_ndarray(double_vec);
}

auto py_object_vec()
{
    return cxtream::python::utility::to_ndarray(object_vec);
}

BOOST_PYTHON_MODULE(pyboost_ndarray_converter_py_cpp)
{
    cxtream::python::initialize();

    py::class_<IntWrap>("IntWrap", py::no_init).def_readonly("num", &IntWrap::num);
    py::def("py_empty_vec",  py_empty_vec);
    py::def("py_char_vec",   py_char_vec);
    py::def("py_int_vec",    py_int_vec);
    py::def("py_double_vec", py_double_vec);
    py::def("py_object_vec", py_object_vec);
}
