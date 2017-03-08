cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project("cxtream")
set(CXTREAM_VERSION_MAJOR 0)
set(CXTREAM_VERSION_MINOR 1)

# -------
# Options
# -------

OPTION(BUILD_TEST "Build test binaries" ON)
OPTION(BUILD_PYTHON "Build C++ <-> Python converters" ON)
OPTION(BUILD_PYTHON_OPENCV "Build C++ <-> Python OpenCV converters (requires BUILD_PYTHON)" ON)

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
if(BUILD_PYTHON)
  set(Python_ADDITIONAL_VERSIONS "3.X")
  find_package(Boost COMPONENTS python3 REQUIRED)
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp REQUIRED)

  if(BUILD_PYTHON_OPENCV)
    find_package(OpenCV COMPONENTS core REQUIRED)
  endif()
endif()

# ---------------------
# Build cxtream::python
# ---------------------

if(BUILD_PYTHON)
  # TODO this should be moved to the .so file targets

  # -------------------------
  # Build cxtream::python .so
  # -------------------------

  add_library(
    cxtream_python SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/src/python/pyboost_initialize.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/python/utility/pyboost_fs_path_converter.cpp"
  )
  target_include_directories(
    cxtream_python
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${PYTHON_INCLUDE_DIRS}
  )
  target_link_libraries(
    cxtream_python
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    stdc++fs
  )
  set(CXTREAM_VERSION_STRING ${CXTREAM_VERSION_MAJOR}.${CXTREAM_VERSION_MINOR})
  set_target_properties(
    cxtream_python
    PROPERTIES VERSION ${CXTREAM_VERSION_STRING}
  )

  # -----------------------------------------------------
  # Build cxtream::python::utility::pyboost_cv3_converter
  # -----------------------------------------------------

  if(BUILD_PYTHON_OPENCV)
    target_sources(
      cxtream_python
      PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/python/utility/pyboost_cv3_converter.cpp"
    )
    target_include_directories(
      cxtream_python
      PRIVATE ${OpenCV_INCLUDE_DIRS}
    )
    target_link_libraries(
      cxtream_python
      ${OpenCV_LIBRARIES}
    )
  endif()

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
