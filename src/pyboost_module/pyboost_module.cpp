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
#include <boost/python.hpp>
#include <range/v3/view/transform.hpp>

#include <cxtream/utility/pyboost_column_converter.hpp>
#include <cxtream/utility/pyboost_cv3_converter.hpp>
#include <cxtream/utility/pyboost_fs_path_converter.hpp>
#include <cxtream/utility/pyboost_range.hpp>

#include STREAM_HEADER_FILE

namespace cxtream {

  namespace p = boost::python;


  std::mt19937 global_prng{std::random_device{}()};


  auto build_python_stream(const fs::path& path)
  {
    auto stream =
        build_stream(path)
      | ranges::view::transform([](auto&& tuple){
          return column_tuple_to_py(std::forward<decltype(tuple)>(tuple));
    });
    return python_iterator<decltype(stream)>{std::move(stream)};
  }


  // the type of the original range
  using range_t = decltype(build_stream(fs::path{}));
  // the type of the python stream to be used as Iterator class in python
  using python_iterator_t = decltype(build_python_stream(fs::path{}));


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

    // expose the iterator class
    p::class_<python_iterator_t>("Iterator", p::no_init)
      .def("__iter__", &python_iterator_t::iter)
      .def("__next__", &python_iterator_t::next);

    // expose the global function to build the stream
    p::def("get_epoch_iterator", build_python_stream);
  }

} //end namespace cxtream
