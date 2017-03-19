/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_UTILITY_STRING_HPP
#define CXTREAM_CORE_UTILITY_STRING_HPP

#include <algorithm>
#include <locale>
#include <locale>
#include <sstream>
#include <string>

#include <range/v3/view/transform.hpp>

namespace cxtream::utility {


  // string_to and to_string are similar to boost::lexical_cast
  // however, they ignore the current locale and use c++11
  // to_string wherever possible


  /* string_to */


  template<typename T>
  T string_to(std::string str)
  {
    std::istringstream ss{std::move(str)};
    ss.imbue(std::locale::classic());
    T val;
    ss >> val;
    if (!ss.eof() || ss.fail()) {
      throw std::ios_base::failure(
        std::string("Failed to read type <") + typeid(T).name() +
        "> from string \"" + ss.str() + "\"");
    }
    return val;
  }

  template<>
  std::string string_to<std::string>(std::string str)
  {
    return str;
  }


  /* to_string */


  // use std::to_string whenever possible
  using std::to_string;

  // add string to string specialization
  std::string to_string(std::string str)
  {
    return str;
  }

  std::string to_string(const char* str)
  {
    return str;
  }

  // add bool to string specialization
  std::string to_string(bool b)
  {
    return b ? "true" : "false";
  }


  /* trim */


  // this one could be taken from boost, but it would be the only
  // dependency in the entire project

  std::string trim(const std::string& str)
  {
     auto isspace = [](char c){ return std::isspace(c, std::locale::classic()); };
     auto begin = std::find_if_not(str.begin(),  str.end(),  isspace);
     auto end   = std::find_if_not(str.rbegin(), str.rend(), isspace).base();
     if (begin < end)
       return std::string(begin, end);
     return std::string{};
  }

}

#endif
