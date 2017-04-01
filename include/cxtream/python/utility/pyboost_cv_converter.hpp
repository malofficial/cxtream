/*********************************************************
 *  pyboost_cv3_converter.hpp
 *
 *  Copyright: (c) 2014, Gregory Kramida
 *  Copyright: (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/


#ifndef CXTREAM_PYTHON_UTILITY_PYBOOST_CV_CONVERTER_HPP
#define CXTREAM_PYTHON_UTILITY_PYBOOST_CV_CONVERTER_HPP

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <Python.h>
#include <boost/python.hpp>
#include <numpy/ndarrayobject.h>
#include <opencv2/core/core.hpp>

#include <cstdio>

namespace cxtream::python::utility {

// standalone converter functions //

PyObject* fromMatToNDArray(const cv::Mat& m);
cv::Mat fromNDArrayToMat(PyObject* o);

// boost converters //

struct matToNDArrayBoostConverter {
    static PyObject* convert(cv::Mat const& m);
};

struct matFromNDArrayBoostConverter {

    matFromNDArrayBoostConverter();

    // check if PyObject is an array and can be converted to OpenCV matrix
    static void* convertible(PyObject* object);

    // construct a Mat from an NDArray object
    static void construct(PyObject* object,
                          boost::python::converter::rvalue_from_python_stage1_data* data);
};

}  // namespace cxtream::python::utility
#endif
