/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#define PY_ARRAY_UNIQUE_SYMBOL pbcvt_ARRAY_API

#include <random>
#include <functional>

#include <boost/python.hpp>

#include <cxtream/utility/pyboost_cv3_converter.hpp>
#include <cxtream/utility/pyboost_fs_path_converter.hpp>

#include STREAM_HEADER_FILE

namespace cxtream {

  namespace p = boost::python;


  std::mt19937 global_prng{std::random_device{}()};


  static void* init_ar()
  {
    Py_Initialize();
    import_array();
    return NUMPY_IMPORT_ARRAY_RETVAL;
  }


  BOOST_PYTHON_MODULE(STREAM_NAME)
  {
    init_ar();

    // register exception for StopIteration
    p::register_exception_translator<stop_iteration_exception>(stop_iteration_translator);

    // register fs::path converter
    p::to_python_converter<fs::path, fs_path_to_python_str>();
    fs_path_from_python_str();

    // register OpenCV converters
    p::to_python_converter<cv::Mat, pbcvt::matToNDArrayBoostConverter>();
    pbcvt::matFromNDArrayBoostConverter();

    p::class_<Dataset>("Dataset", p::init<fs::path>())
      .def("create_train_stream", &Dataset::create_train_stream_py)
      .def("create_valid_stream", &Dataset::create_valid_stream_py)
      .def("create_test_stream", &Dataset::create_valid_stream_py);
  }

} //end namespace cxtream
