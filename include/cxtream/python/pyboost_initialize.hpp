/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_PYTHON_PYBOOST_INITIALIZE_HPP
#define CXTREAM_PYTHON_PYBOOST_INITIALIZE_HPP

namespace cxtream::python {

/// Initialize Python module, register OpenCV converters, exceptions, etc.
void initialize();

}  // namespace cxtream::python
#endif
