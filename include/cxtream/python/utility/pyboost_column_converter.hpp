/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_PYTHON_UTILITY_PYBOOST_COLUMN_CONVERTER_HPP
#define CXTREAM_PYTHON_UTILITY_PYBOOST_COLUMN_CONVERTER_HPP

#include <string>
#include <vector>

#include <boost/python.hpp>

#include <cxtream/core/utility/tuple.hpp>

namespace cxtream::python::utility {


  /* recursive transformation from a multidimensional vector to a python list */

  namespace detail {

    template<typename T>
    struct vector_to_py_impl
    {
      template<typename U>
      static U&& impl(U&& val)
      {
        return std::forward<U>(val);
      }
    };

    template<typename T>
    struct vector_to_py_impl<std::vector<T>>
    {
      static boost::python::list impl(std::vector<T> vec)
      {
        boost::python::list res;
        for (auto& val : vec) {
          res.append(vector_to_py_impl<T>::impl(std::move(val)));
        }
        return res;
      }
    };

  } // end namespace

  template<typename Vector>
  boost::python::list vector_to_py(Vector&& v)
  {
    return detail::vector_to_py_impl<std::decay_t<Vector>>::impl(
      std::forward<Vector>(v));
  }

  /* transformation from a column tuple to a python dict */


  template<typename Tuple>
  boost::python::dict column_tuple_to_py(Tuple tuple)
  {
    boost::python::dict res;
    cxtream::utility::tuple_for_each([&res](auto& column){
      res[column.name] = vector_to_py(std::move(column.value));
    }, tuple);
    return res;
  }


} //end namespace cxtream::python::utility
#endif
