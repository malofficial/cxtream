/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#ifndef CXTREAM_PYBOOST_RANGE_UTILS_HPP
#define CXTREAM_PYBOOST_RANGE_UTILS_HPP

#include <range/v3/core.hpp>
#include <boost/python.hpp>

namespace cxtream {


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
  class python_iterator {
    using iterator_t = decltype(ranges::begin(std::declval<Rng&>()));
    using sentinel_t = decltype(ranges::end(std::declval<Rng&>()));

    iterator_t iterator_;
    sentinel_t sentinel_;
    Rng rng_;

    public:
      python_iterator() = default;

      explicit python_iterator(Rng rng)
        : rng_{std::move(rng)}
      {
        initialize_iterators();
      }

      python_iterator(const python_iterator& rhs)
        : rng_{rhs.rng_}
      {
        initialize_iterators();
      }

      python_iterator<Rng> & operator=(const python_iterator& rhs)
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

} //end namespace cxtream
#endif
