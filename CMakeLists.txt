cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project("cxtream")

# -------
# Options
# -------

OPTION(BUILD_PYTHON_LIB "Build Python dynamic library" ON)
OPTION(BUILD_PYTHON_WHEEL "Build Python wheel package" ON)
OPTION(BUILD_TEST "Build test binaries" ON)
OPTION(BUILD_PYTHON "Build C++ <-> Python converters" ON)
OPTION(BUILD_PYTHON_OPENCV "Build C++ <-> Python OpenCV converters (requires WITH_PYTHON)" ON)

# -------------
# CMake Options
# -------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# -------------------------------------
# Dump CMake Options Into a Header File
# -------------------------------------

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cxtream/build_config.hpp.in
  ${CMAKE_BINARY_DIR}/include/cxtream/build_config.hpp
)

# --------------
# Compiler Flags
# --------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wno-missing-braces -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")

# ---------------------
# Find Common Libraries
# ---------------------

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_BINARY_DIR}/include")
# TODO this should be moved to the .so file targets
if(BUILD_PYTHON_OPENCV)
  find_package(OpenCV COMPONENTS core REQUIRED)
endif()

# --------------
# Build Streams
# --------------

include("AddPythonStream")
add_subdirectory("stream")

# -----
# Tests
# -----

if(BUILD_TEST)
  find_package(Boost COMPONENTS unit_test_framework REQUIRED)
  enable_testing()
  include("BoostTest")
  add_subdirectory("test")
endif()
