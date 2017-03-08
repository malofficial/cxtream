/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_PYTHON_PYBOOST_RANGE_ITERATOR_HPP
#define CXTREAM_PYTHON_PYBOOST_RANGE_ITERATOR_HPP

#include <functional>
#include <string>
#include <typeinfo>

#include <range/v3/core.hpp>
#include <range/v3/view/transform.hpp>
#include <boost/python.hpp>

#include <cxtream/python/utility/pyboost_column_converter.hpp>

namespace cxtream::python {


  struct stop_iteration_exception: public std::runtime_error
  {
    stop_iteration_exception()
      : std::runtime_error{"stop iteration"}
    {}
  };


  void stop_iteration_translator(const stop_iteration_exception& x) {
    PyErr_SetNone(PyExc_StopIteration);
  }


  template<typename Rng>
  class stream_iterator {
    using iterator_t = decltype(ranges::begin(std::declval<Rng&>()));
    using sentinel_t = decltype(ranges::end(std::declval<Rng&>()));

    iterator_t iterator_;
    sentinel_t sentinel_;
    Rng rng_;

    public:
      stream_iterator() = default;

      explicit stream_iterator(Rng rng)
        : rng_{std::move(rng)}
      {
        initialize_iterators();

        namespace py = boost::python;
        // TODO call only once!

        // register exception for StopIteration
        py::register_exception_translator<stop_iteration_exception>(stop_iteration_translator);

        // register python iterator type
        using this_t = stream_iterator<Rng>;
        std::string this_t_name = std::string("cxtream_") + typeid(this_t).name();
        py::class_<this_t>(this_t_name.c_str(), py::no_init)
          .def("__iter__", &this_t::iter)
          .def("__next__", &this_t::next);
      }

      stream_iterator(const stream_iterator& rhs)
        : rng_{rhs.rng_}
      {
        initialize_iterators();
      }

      stream_iterator<Rng> & operator=(const stream_iterator& rhs)
      {
        rng_ = rhs.rng_;
        initialize_iterators();
      }

      void initialize_iterators()
      {
        iterator_ = ranges::begin(rng_);
        sentinel_ = ranges::end(rng_);
      }

      auto iter()
      {
        return *this;
      }

      auto next()
      {
        if (iterator_ == sentinel_)
          throw stop_iteration_exception();
        auto val = *iterator_;
        ++iterator_;
        return val;
      }
  };


  // until the compiler has full support for C++17 template argument deduction
  template<typename Rng>
  auto make_iterator(Rng&& rng)
  {
    // transform the range of columns to a range of python types
    auto range_of_python_types =
        std::forward<Rng&&>(rng)
      | ranges::view::transform([](auto&& tuple){
          return utility::column_tuple_to_py(std::forward<decltype(tuple)>(tuple));
        });

    // make python iterator out of the range of python types
    using PyRng = decltype(range_of_python_types);
    return stream_iterator<PyRng>{std::move(range_of_python_types)};
  }


} // end namespace cxtream::python
#endif
