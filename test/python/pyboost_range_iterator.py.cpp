/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#include <cxtream/python/pyboost_initialize.hpp>
#include <cxtream/python/pyboost_range_iterator.hpp>

#include <list>
#include <vector>

namespace py = boost::python;
namespace cxpy = cxtream::python;

using list_iter_t = cxpy::iterator<std::list<long>>;
list_iter_t empty_iterator()
{
    return list_iter_t{std::list<long>{}};
}

using vec_iter_t = cxpy::iterator<std::vector<long>>;
vec_iter_t fib_iterator()
{
    return vec_iter_t{std::vector<long>{1, 1, 2, 3, 5, 8}};
}

BOOST_PYTHON_MODULE(pyboost_range_iterator_py_cpp)
{
    // initialize cxtream OpenCV converters, exceptions, etc.
    cxtream::python::initialize();

    // expose the functions
    py::def("empty_iterator", empty_iterator);
    py::def("fib_iterator", fib_iterator);
}
