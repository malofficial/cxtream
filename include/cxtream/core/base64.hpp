/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_BASE64_HPP
#define CXTREAM_CORE_BASE64_HPP

#include <vector>
#include <string>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

namespace cxtream {


  /* decode base64 string */


  std::vector<std::uint8_t> base64_decode(const std::string& b64data)
  {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;

    // skip padding characters
    std::size_t len = b64data.size();
    while (len && b64data[len - 1] == '=') --len;

    return std::vector<std::uint8_t>(It(std::begin(b64data)), It(std::begin(b64data) + len));
  }


  /* encode base64 string */


  std::string base64_encode(const std::vector<std::uint8_t>& data)
  {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::vector<std::uint8_t>::const_iterator, 6, 8>>;
    std::string res(It(std::begin(data)), It(std::end(data)));
    return res.append((3 - data.size() % 3) % 3, '=');
  }


} // end namespace cxtream
#endif
