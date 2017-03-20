/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#include <cxtream/python.hpp>
#include "mnist_stream.hpp"

namespace py = boost::python;
using namespace mnist_stream;


/* Add python specific functions to the dataset */


struct DatasetPy : Dataset
{

  using Dataset::Dataset;


  auto create_train_stream_py()
  {
    return cxtream::python::make_iterator(train_stream());
  }


  auto create_valid_stream_py()
  {
    return cxtream::python::make_iterator(valid_stream());
  }


  auto create_test_stream_py()
  {
    return cxtream::python::make_iterator(test_stream());
  }

};


/* Export the dataset as a python module */


BOOST_PYTHON_MODULE(mnist_stream)
{
  cxtream::python::initialize();

  py::class_<DatasetPy>("Dataset", py::init<fs::path>())
    .def("create_train_stream", &DatasetPy::create_train_stream_py)
    .def("create_valid_stream", &DatasetPy::create_valid_stream_py)
    .def("create_test_stream", &DatasetPy::create_valid_stream_py)
    .def("split", &DatasetPy::split);
}
