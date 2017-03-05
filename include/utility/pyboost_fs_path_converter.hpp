/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_UTILITY_PYBOOST_FS_PATH_CONVERTER_HPP
#define CXTREAM_UTILITY_PYBOOST_FS_PATH_CONVERTER_HPP

#include <boost/python.hpp>
#include <experimental/filesystem>

namespace cxtream {

	namespace fs = std::experimental::filesystem;
  namespace p = boost::python;


  struct fs_path_to_python_str
  {
    static PyObject* convert(const fs::path& path)
    {
      return p::incref(p::object(path.string()).ptr());
    }
  };


  struct fs_path_from_python_str
  {
    fs_path_from_python_str()
    {
      boost::python::converter::registry::push_back(
        &convertible,
        &construct,
        boost::python::type_id<fs::path>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
      if (!PyUnicode_Check(obj_ptr)) return 0;
      return obj_ptr;
    }

    static void construct(
      PyObject* obj_ptr,
      boost::python::converter::rvalue_from_python_stage1_data* data)
    {
      const char* value = PyUnicode_AsUTF8(obj_ptr);
      if (value == 0) boost::python::throw_error_already_set();
      void* storage = (
        (boost::python::converter::rvalue_from_python_storage<fs::path>*)
          data)->storage.bytes;
      new (storage) fs::path(value);
      data->convertible = storage;
    }
  };


} //end namespace cxtream
#endif
