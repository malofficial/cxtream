/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#include <cxtream/build_config.hpp>
#include <cxtream/python/initialize.hpp>
#include <cxtream/python/range.hpp>

#ifdef CXTREAM_BUILD_PYTHON_OPENCV
// the header file for python::initialize sets NO_IMPORT_ARRAY
// but we actually really import_array here (this is the only place), so unset it.
#undef NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL CXTREAM_PYTHON_UTILITY_PYBOOST_CV_CONVERTER
#include <cxtream/python/utility/pyboost_cv_converter.hpp>
#endif

#include <cxtream/python/utility/pyboost_fs_path_converter.hpp>

#include <boost/python.hpp>

namespace cxtream::python {

#ifdef CXTREAM_BUILD_PYTHON_OPENCV
static void* init_array()
{
    import_array();
    return NUMPY_IMPORT_ARRAY_RETVAL;
}
#endif

void initialize()
{
    namespace py = boost::python;

    // initialize python module
    Py_Initialize();

    // register stop_iteration_exception
    py::register_exception_translator<stop_iteration_exception>(stop_iteration_translator);

#ifdef CXTREAM_BUILD_PYTHON_OPENCV
    // initialize numpy array
    init_array();

    // register OpenCV converters
    py::to_python_converter<cv::Mat, utility::matToNDArrayBoostConverter>();
    utility::matFromNDArrayBoostConverter();
#endif

    // register fs::path converter
    py::to_python_converter<std::experimental::filesystem::path, utility::fs_path_to_python_str>();
    utility::fs_path_from_python_str();
}

}  // namespace cxtream::python
