/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AUGMENTER_HPP
#define AUGMENTER_HPP

#include <type_traits>
#include <range/v3/view.hpp>

namespace stream {


  namespace detail {


  } // end namespace detail


  template<typename... Columns>
  struct From {};


  template<typename... Columns>
  struct To {};


  template<typename<typename... FromColumns> From,
           typename<typename... ToColumns> To>
  struct augmenter
  {
    template<typename Rng, typename Fun>
    auto operator(Rng && rng, Fun fun) const {
      return view::transform([](auto Tuple){
        
      });
    }
  };


} // end namespace stream
#endif
