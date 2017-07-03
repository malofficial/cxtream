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
#include <cxtream/python/stream/iterator.hpp>

#include <list>
#include <vector>

namespace py = boost::python;
namespace cxpy = cxtream::python;

CXTREAM_DEFINE_COLUMN(Int, int)
CXTREAM_DEFINE_COLUMN(Double, double)

std::vector<std::tuple<Int, Double>> empty_data;
std::vector<std::tuple<Int, Double>> empty_batch_data(1);
const std::list<std::tuple<Int, Double>> number_data = {{{3, 2}, 5.}, {{1, 4}, 2.}};

auto empty_iterator()
{
    return cxpy::stream::make_iterator(empty_data);
}

auto empty_batch_iterator()
{
    return cxpy::stream::make_iterator(empty_batch_data);
}

auto number_iterator()
{
    return cxpy::stream::make_iterator(number_data);
}

BOOST_PYTHON_MODULE(iterator_py_cpp)
{
    // initialize cxtream OpenCV converters, exceptions, etc.
    cxtream::python::initialize();

    // expose the functions
    py::def("empty_iterator", empty_iterator);
    py::def("empty_batch_iterator", empty_batch_iterator);
    py::def("number_iterator", number_iterator);
}
