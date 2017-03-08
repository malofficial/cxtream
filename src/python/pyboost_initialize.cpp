/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#include <cxtream/build_config.hpp>

#include <boost/python.hpp>

#include <cxtream/python/pyboost_initialize.hpp>

#ifdef CXTREAM_BUILD_PYTHON_OPENCV
#define PY_ARRAY_UNIQUE_SYMBOL pbcvt_ARRAY_API
#include <cxtream/python/utility/pyboost_cv3_converter.hpp>
#endif

#include <cxtream/python/utility/pyboost_fs_path_converter.hpp>

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
    py::to_python_converter<cv::Mat, pbcvt::matToNDArrayBoostConverter>();
    pbcvt::matFromNDArrayBoostConverter();
#endif

    // register fs::path converter
    py::to_python_converter<std::experimental::filesystem::path,
                           utility::fs_path_to_python_str>();
    utility::fs_path_from_python_str();
  }


} //end namespace cxtream::python
