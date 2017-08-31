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

#include <cxtream/python/utility/pyboost_is_registered.hpp>

#include <boost/python.hpp>
#include <range/v3/core.hpp>

#include <memory>
#include <stdexcept>
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
    using this_t     = iterator<Rng>;

    std::shared_ptr<Rng> rng_ptr_;
    iterator_t iterator_;
    bool first_iteration_;

    // register __len__ function if it is supported
    CONCEPT_REQUIRES(ranges::SizedRange<const Rng>())
    static void register_iterator_len(boost::python::class_<this_t>& cls)
    {
        cls.def("__len__", &this_t::len);
    }
    CONCEPT_REQUIRES(!ranges::SizedRange<const Rng>())
    static void register_iterator_len(boost::python::class_<this_t>&)
    {
    }

    // register __getitem__ function if it is supported
    CONCEPT_REQUIRES(ranges::RandomAccessRange<const Rng>())
    static void register_iterator_getitem(boost::python::class_<this_t>& cls)
    {
        cls.def("__getitem__", &this_t::getitem);
    }
    CONCEPT_REQUIRES(!ranges::RandomAccessRange<const Rng>())
    static void register_iterator_getitem(boost::python::class_<this_t>&)
    {
    }

    // function to register the type of this class in boost::python
    // makes sure the type is registered only once
    static void register_iterator()
    {
        namespace py = boost::python;

        if (!utility::is_registered<this_t>()) {
            std::string this_t_name = std::string("cxtream_") + typeid(this_t).name();
            py::class_<this_t> cls{this_t_name.c_str(), py::no_init};
            cls.def("__iter__", &this_t::iter);
            cls.def("__next__", &this_t::next);
            register_iterator_len(cls);
            register_iterator_getitem(cls);
        };
    }

public:
    iterator() = default;

    /// Construct iterator from a range.
    explicit iterator(Rng rng)
      : rng_ptr_{std::make_shared<Rng>(std::move(rng))},
        iterator_{ranges::begin(*rng_ptr_)},
        first_iteration_{true}
    {
        register_iterator();
    }

    /// Construct iterator from std::shared_ptr.
    iterator(std::shared_ptr<Rng> rng_ptr)
      : rng_ptr_{std::move(rng_ptr)},
        iterator_{ranges::begin(*rng_ptr_)},
        first_iteration_{true}
    {
        register_iterator();
    }

    /// Return a copy of this iterator restarted to the beginning of the range.
    iterator<Rng> iter()
    {
        return iterator<Rng>{rng_ptr_};
    }

    /// Return the next element in the range.
    ///
    /// \throws stop_iteration_exception if there are no more elements.
    auto next()
    {
        // do not increment the iterator in the first iteration, just return *begin()
        if (!first_iteration_ && iterator_ != ranges::end(*rng_ptr_)) ++iterator_;
        first_iteration_ = false;
        if (iterator_ == ranges::end(*rng_ptr_)) throw stop_iteration_exception();
        return *iterator_;
    }

    /// Get an item or a slice.
    ///
    /// Note that when slicing, the data get copied.
    CONCEPT_REQUIRES(ranges::RandomAccessRange<const Rng>())
    boost::python::object getitem(PyObject* idx_py) const
    {
        // handle slices
        if (PySlice_Check(idx_py)) {
            PySliceObject* slice = static_cast<PySliceObject*>(static_cast<void*>(idx_py));
            if (slice->step != Py_None) {
                throw std::logic_error("Cxtream python iterator does not support slice steps.");
            }

            auto handle_index = [this](PyObject* idx_py, long def_val) {
                long idx = def_val;
                if (idx_py != Py_None) {
                    idx = boost::python::extract<long>(idx_py);
                    // reverse negative index
                    if (idx < 0) idx += this->len();
                    // if it is still negative, clip
                    if (idx < 0) idx = 0;
                    // handle index larger then len
                    if (idx > this->len()) idx = len();
                }
                return idx;
            };
            long start = handle_index(slice->start, 0);
            long stop = handle_index(slice->stop, len());
            if (start > stop) start = stop;

            using slice_data_type = std::vector<ranges::range_value_type_t<Rng>>;
            slice_data_type slice_data{rng_ptr_->begin() + start, rng_ptr_->begin() + stop};
            return boost::python::object{iterator<slice_data_type>{std::move(slice_data)}};
        }

        // handle indices
        long idx = boost::python::extract<long>(idx_py);
        if (idx < 0) idx += len();
        return boost::python::object{ranges::at(*rng_ptr_, idx)};
    }

    /// Get the size of the range.
    CONCEPT_REQUIRES(ranges::SizedRange<const Rng>())
    long len() const
    {
        return ranges::size(*rng_ptr_);
    }

};  // class iterator

}  // end namespace cxtream::python
#endif
