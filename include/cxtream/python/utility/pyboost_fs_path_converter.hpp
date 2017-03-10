/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
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
