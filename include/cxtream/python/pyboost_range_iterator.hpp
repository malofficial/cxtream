/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_PYTHON_PYBOOST_RANGE_ITERATOR_HPP
#define CXTREAM_PYTHON_PYBOOST_RANGE_ITERATOR_HPP

#include <functional>
#include <mutex>
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


  namespace detail {

    void stop_iteration_translator(const stop_iteration_exception& x) {
      PyErr_SetNone(PyExc_StopIteration);
    }

    // function to register exception for StopIteration
    // the exception type is only registered once
    std::once_flag register_stop_iteration_flag;
    void register_stop_iterator()
    {
      namespace py = boost::python;
      std::call_once(
        register_stop_iteration_flag,
        [](){
          py::register_exception_translator
            <stop_iteration_exception>(stop_iteration_translator);
      });
    }

  }

  template<typename Rng>
  class stream_iterator {
    using iterator_t = decltype(ranges::begin(std::declval<Rng&>()));
    using sentinel_t = decltype(ranges::end(std::declval<Rng&>()));

    iterator_t iterator_;
    sentinel_t sentinel_;
    Rng rng_;

    // add function to register this type in boost python
    // make sure the type is registered only once
    static std::once_flag register_flag;
    static void register_iterator()
    {
      namespace py = boost::python;
      using this_t = stream_iterator<Rng>;

      detail::register_stop_iterator();
      std::call_once(
        register_flag,
        [](){
          std::string this_t_name = std::string("cxtream_") + typeid(this_t).name();
          py::class_<this_t>(this_t_name.c_str(), py::no_init)
            .def("__iter__", &this_t::iter)
            .def("__next__", &this_t::next);
      });
    }


    public:

      stream_iterator() = default;

      explicit stream_iterator(Rng rng)
        : rng_{std::move(rng)}
      {
        initialize_iterators();
        register_iterator();
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

  // provide definition for the static register_flag variable
  template<typename Rng>
  std::once_flag stream_iterator<Rng>::register_flag;

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
