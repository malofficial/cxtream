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

#include <range/v3/view/transform.hpp>

#include <algorithm>
#include <locale>
#include <locale>
#include <sstream>
#include <string>

namespace cxtream::utility {

/// Convert std::string to a given type.
///
/// This function is similar to boost::lexical_cast,
/// however, it always uses C locale.
template<typename T>
T string_to(std::string str)
{
    std::istringstream ss{std::move(str)};
    ss.imbue(std::locale::classic());
    T val;
    ss >> val;
    if (!ss.eof() || ss.fail()) {
        throw std::ios_base::failure(std::string("Failed to read type <") + typeid(T).name() +
                                     "> from string \"" + ss.str() + "\"");
    }
    return val;
}

template<>
std::string string_to<std::string>(std::string str)
{
    return str;
}

/// Convert std::string to a given type.
///
/// This function is similar to boost::lexical_cast,
/// however, it always uses C locale and uses c++11
/// to_string wherever possible.
using std::to_string;

std::string to_string(std::string str)
{
    return str;
}

std::string to_string(const char* str)
{
    return str;
}

std::string to_string(bool b)
{
    return b ? "true" : "false";
}

/// Removes whitespace characters from the beginning and the end of a string.
///
/// This function is similar to boost::trimmed,
/// however, it always uses C locale.
std::string trim(const std::string& str)
{
    auto isspace = [](char c) { return std::isspace(c, std::locale::classic()); };
    auto begin = std::find_if_not(str.begin(), str.end(), isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), isspace).base();
    if (begin < end) return std::string(begin, end);
    return std::string{};
}

}  // namespace cxtream
#endif
