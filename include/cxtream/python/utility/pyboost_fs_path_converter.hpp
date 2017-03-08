/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_PYTHON_UTILITY_PYBOOST_FS_PATH_CONVERTER_HPP
#define CXTREAM_PYTHON_UTILITY_PYBOOST_FS_PATH_CONVERTER_HPP

#include <experimental/filesystem>

#include <boost/python.hpp>

namespace cxtream::python::utility {

  namespace {
    namespace fs = std::experimental::filesystem;
    namespace py = boost::python;
  }


  struct fs_path_to_python_str
  {
    static PyObject* convert(const fs::path& path);
  };


  struct fs_path_from_python_str
  {
    fs_path_from_python_str();

    static void* convertible(PyObject* obj_ptr);

    static void construct(
      PyObject* obj_ptr,
      py::converter::rvalue_from_python_stage1_data* data);
  };


} //end namespace cxtream::python::utility
#endif
