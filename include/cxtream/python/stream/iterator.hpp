/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_PYTHON_STREAM_ITERATOR_HPP
#define CXTREAM_PYTHON_STREAM_ITERATOR_HPP

#include <cxtream/python/pyboost_range_iterator.hpp>
#include <cxtream/python/utility/pyboost_column_converter.hpp>

#include <range/v3/view/transform.hpp>

namespace cxtream::python::stream {

/// Make Python iterator for a stream (i.e, a range of tuples of columns).
///
/// Beware: The iterator represents only a view of the provided range! Be careful
/// about the lifetime of the provided range.
/// Columns represented by n-dimensional std::vectors are automatically
/// converted to n-dimensional python lists.
template <typename Rng>
auto make_iterator(Rng& rng)
{
    // transform the range of columns to a range of python types
    auto range_of_dicts = rng
      | ranges::view::transform([](auto&& tuple) {
            return utility::columns_to_dict(std::forward<decltype(tuple)>(tuple));
        });

    // make python iterator out of the range of python types
    using PyRng = decltype(range_of_dicts);
    return iterator<PyRng>{std::move(range_of_dicts)};
}

}  // end namespace cxtream::python::stream
#endif
