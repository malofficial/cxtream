/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_PYTHON_PYBOOST_INITIALIZE_HPP
#define CXTREAM_PYTHON_PYBOOST_INITIALIZE_HPP

#include <cxtream/build_config.hpp>

#ifdef CXTREAM_BUILD_PYTHON_OPENCV
#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL CXTREAM_PYTHON_UTILITY_PYBOOST_CV_CONVERTER
#endif

namespace cxtream::python {

/// Initialize Python module, register OpenCV converters, exceptions, etc.
void initialize();

}  // namespace cxtream::python
#endif
