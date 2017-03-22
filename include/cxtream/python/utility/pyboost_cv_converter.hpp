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

#include <cstdio>

#include <boost/python.hpp>
#include <numpy/ndarrayobject.h>
#include <opencv2/core/core.hpp>
#include <Python.h>

namespace pbcvt {

using namespace cv;


//===================    MACROS    =================================================================
#define ERRWRAP2(expr) \
try \
{ \
    PyAllowThreads allowThreads; \
    expr; \
} \
catch (const cv::Exception &e) \
{ \
    PyErr_SetString(opencv_error, e.what()); \
    return 0; \
}

//===================   ERROR HANDLING     =========================================================

// static int failmsg(const char *fmt, ...);
// static PyObject* failmsgp(const char *fmt, ...);

//===================   THREADING     ==============================================================
class PyAllowThreads;
class PyEnsureGIL;

static size_t REFCOUNT_OFFSET = (size_t)&(((PyObject*)0)->ob_refcnt) +
    (0x12345678 != *(const size_t*)"\x78\x56\x34\x12\0\0\0\0\0")*sizeof(int);

static inline PyObject* pyObjectFromRefcount(const int* refcount)
{
    return (PyObject*)((size_t)refcount - REFCOUNT_OFFSET);
}

static inline int* refcountFromPyObject(const PyObject* obj)
{
    return (int*)((size_t)obj + REFCOUNT_OFFSET);
}

//===================   NUMPY ALLOCATOR FOR OPENCV     =============================================

class NumpyAllocator;

//===================   STANDALONE CONVERTER FUNCTIONS     =========================================

PyObject* fromMatToNDArray(const Mat& m);
Mat fromNDArrayToMat(PyObject* o);

//===================   BOOST CONVERTERS     =======================================================

struct matToNDArrayBoostConverter {
	static PyObject* convert(Mat const& m);
};


struct matFromNDArrayBoostConverter {

	matFromNDArrayBoostConverter();

	/// @brief Check if PyObject is an array and can be converted to OpenCV matrix.
	static void* convertible(PyObject* object);

	/// @brief Construct a Mat from an NDArray object.
	static void construct(PyObject* object,
			boost::python::converter::rvalue_from_python_stage1_data* data);
};

} // end namespace pbcvt
#endif
