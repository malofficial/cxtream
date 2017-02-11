/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#define PY_ARRAY_UNIQUE_SYMBOL pbcvt_ARRAY_API

#include <utility/pyboost_cv3_converter.hpp>
#include <utility/pyboost_range.hpp>
#include <stream.hpp>

#include <random>
#include <boost/python.hpp>

namespace stream {

  namespace p = boost::python;


  std::mt19937 global_prng{std::random_device{}()};


  auto build_python_stream()
  {
    using Stream = decltype(build_stream()); // until C++17 (and GCC 7)
    return python_iterator<Stream>{build_stream()};
  }
  

  // the type of the python stream to be used as Iterator class in python
  using python_iterator_t = decltype(build_python_stream());


  static void * init_ar()
  {
    Py_Initialize();
    import_array();
    return NUMPY_IMPORT_ARRAY_RETVAL;
  }


  BOOST_PYTHON_MODULE(stream)
  {
    init_ar();

    // register exception for StopIteration
    p::register_exception_translator<stop_iteration_exception>(stop_iteration_translator);

    // initialize OpenCV converters
    p::to_python_converter<cv::Mat, pbcvt::matToNDArrayBoostConverter>();
    pbcvt::matFromNDArrayBoostConverter();
    
    // expose the iterator class
    p::class_<python_iterator_t>("Iterator", p::no_init)
      .def("__iter__", &python_iterator_t::iter)
      .def("__next__", &python_iterator_t::next);

    // expose the global function to build the stream
    p::def("get_epoch_iterator", build_python_stream);
  }

} //end namespace stream
