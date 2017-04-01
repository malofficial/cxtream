/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#include <cxtream/build_config.hpp>
#include <cxtream/python/pyboost_initialize.hpp>

#ifdef CXTREAM_BUILD_PYTHON_OPENCV
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
