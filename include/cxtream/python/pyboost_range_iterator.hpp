/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_PYTHON_PYBOOST_RANGE_ITERATOR_HPP
#define CXTREAM_PYTHON_PYBOOST_RANGE_ITERATOR_HPP

#include <boost/python.hpp>
#include <range/v3/core.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <typeinfo>

namespace cxtream::python {

/// Exception which is translated to Python's StopIteration when thrown.
struct stop_iteration_exception : public std::runtime_error {
    stop_iteration_exception()
      : std::runtime_error{"stop iteration"}
    { }
};

/// boost::python translation function for the stop_iteration_exception.
void stop_iteration_translator(const stop_iteration_exception& x)
{
    PyErr_SetNone(PyExc_StopIteration);
}

/// Python adapter for C++ ranges.
///
/// This class provides __next__ and __iter__ methods emulating python iterators.
template<typename Rng>
class iterator {
private:
    using iterator_t = decltype(ranges::begin(std::declval<Rng&>()));
    using sentinel_t = decltype(ranges::end(std::declval<Rng&>()));

    std::shared_ptr<Rng> rng_ptr_;
    iterator_t iterator_;
    bool first_iteration_;

    // function to register the type of this class in boost::python
    // makes sure the type is registered only once
    static std::once_flag register_flag;
    static void register_iterator()
    {
        namespace py = boost::python;
        using this_t = iterator<Rng>;

        std::call_once(register_flag, []() {
            std::string this_t_name = std::string("cxtream_") + typeid(this_t).name();
            py::class_<this_t>(this_t_name.c_str(), py::no_init)
              .def("__iter__", &this_t::iter)
              .def("__next__", &this_t::next);
        });
    }

public:
    iterator() = default;

    explicit iterator(Rng rng)
      : rng_ptr_{std::make_shared<Rng>(std::move(rng))},
        iterator_{ranges::begin(*rng_ptr_)},
        first_iteration_{true}
    {
        register_iterator();
    }

    iterator<Rng> iter()
    {
        return *this;
    }

    auto next()
    {
        // do not increment the iterator in the first iteration, just return *begin()
        if (!first_iteration_ && iterator_ != ranges::end(*rng_ptr_)) ++iterator_;
        first_iteration_ = false;
        if (iterator_ == ranges::end(*rng_ptr_)) throw stop_iteration_exception();
        return *iterator_;
    }

};  // class iterator

// provide definition for the static register_flag variable
template<typename Rng>
std::once_flag iterator<Rng>::register_flag;

}  // end namespace cxtream::python
#endif
