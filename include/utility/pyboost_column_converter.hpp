/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef STREAM_UTILITY_PYBOOST_COLUMN_CONVERTER_HPP
#define STREAM_UTILITY_PYBOOST_COLUMN_CONVERTER_HPP

#include <utility/tuple.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <string>
#include <vector>
#include <stdexcept>
#include <typeinfo>

namespace stream {

  namespace p = boost::python;


  /* recursive transformation from a multidimensional vector to a python list */


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
    static p::list impl(std::vector<T> vec)
    {
      p::list res;
      for (auto& val : vec) {
        res.append(vector_to_py_impl<T>::impl(std::move(val)));
      }
      return res;
    }
  };

  template<typename Vector>
  p::list vector_to_py(Vector&& v)
  {
    return vector_to_py_impl<std::decay_t<Vector>>::impl(
      std::forward<Vector>(v));
  }

  /* transformation from a column tuple to a python dict */


  template<typename Tuple>
  p::dict column_tuple_to_py(Tuple tuple)
  {
    p::dict res;
    utility::tuple_for_each([&res](auto& column){
      res[column.name] = vector_to_py(std::move(column.value));
    }, tuple);
    return res;
  };


} //end namespace stream
#endif
