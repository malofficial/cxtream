/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/
// TODO compile this file and also fs_path_converter

#ifndef CXTREAM_PYTHON_PYBOOST_INITIALIZE_HPP
#define CXTREAM_PYTHON_PYBOOST_INITIALIZE_HPP

#include <cxtream/build_config.hpp>

#include <boost/python.hpp>

#ifdef CXTREAM_HAS_PYTHON_OPENCV
#define PY_ARRAY_UNIQUE_SYMBOL pbcvt_ARRAY_API
#include <cxtream/python/utility/pyboost_cv3_converter.hpp>
#endif

#include <cxtream/python/utility/pyboost_fs_path_converter.hpp>

namespace cxtream::python {


  void initialize()
  {
    namespace p = boost::python;

    // initialize python module
    Py_Initialize();

#ifdef CXTREAM_HAS_PYTHON_OPENCV
    // initialize numpy array
    import_array();

    // register OpenCV converters
    p::to_python_converter<cv::Mat, pbcvt::matToNDArrayBoostConverter>();
    pbcvt::matFromNDArrayBoostConverter();
#endif

    // register fs::path converter
    p::to_python_converter<std::experimental::filesystem::path,
                           utility::fs_path_to_python_str>();
    utility::fs_path_from_python_str();
  }


} //end namespace cxtream::python
#endif
