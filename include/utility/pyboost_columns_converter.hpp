/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef STREAM_UTILITY_PYBOOST_COLUMNS_CONVERTER_HPP
#define STREAM_UTILITY_PYBOOST_COLUMNS_CONVERTER_HPP

#include <utility/tuple.hpp>
#include <boost/python.hpp>

namespace stream {

  namespace p = boost::python;


  template<typename Tuple>
  p::dict column_tuple_to_py(Tuple&& tuple)
  {
    p::dict res;
    utility::tuple_for_each([&res](auto&& column){
      res[column.name] = std::forward<decltype(column)>(column).value;
    }, std::forward<Tuple>(tuple));
    return res;
  };


} //end namespace stream
#endif
