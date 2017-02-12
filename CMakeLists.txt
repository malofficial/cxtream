cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project("stream")

# -------
# Options
# -------

OPTION(BUILD_PYTHON_LIB "Build Python dynamic library" ON)
OPTION(BUILD_PYTHON_WHEEL "Build Python wheel package" ON)
OPTION(BUILD_TEST "Build test binaries" ON)

# -------------
# CMake Options
# -------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# --------------
# Compiler Flags
# --------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wno-missing-braces -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")

# ---------------------
# Find Common Libraries
# ---------------------

find_package(OpenCV COMPONENTS core highgui imgproc REQUIRED)
find_package(Boost COMPONENTS python3 unit_test_framework REQUIRED)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# -------------
# Python Stream
# -------------

if(BUILD_PYTHON_LIB)

  # find python packages
  set(Python_ADDITIONAL_VERSIONS "3.X")
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp REQUIRED)
  include("FindPythonSuffix")

  # build stream dynamic library
  file(
    GLOB_RECURSE pyboost_module_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pyboost_module/*.cpp")
  add_library(
    ${PROJECT_NAME}_python_lib SHARED
    ${pyboost_module_sources}
  )
  target_include_directories(
    ${PROJECT_NAME}_python_lib
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ${PYTHON_INCLUDE_DIRS}
  )
  set_target_properties(
    ${PROJECT_NAME}_python_lib PROPERTIES
    PREFIX ""
    OUTPUT_NAME ${PROJECT_NAME}
    SUFFIX ${PYTHON_SUFFIX}
  )
  target_link_libraries(
    ${PROJECT_NAME}_python_lib
    ${Boost_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${PYTHON_LIBRARIES}
    stdc++fs
  )

  # ----------------------
  # Python wheel installer
  # ----------------------

  if(BUILD_PYTHON_WHEEL)

    add_custom_target(
      ${PROJECT_NAME}_python_wheel ALL
      COMMAND ${PYTHON_EXECUTABLE} "setup.py" "bdist_wheel"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      DEPENDS ${PROJECT_NAME}_python_lib
    )

  endif()
endif()

# -----
# Tests
# -----

if(BUILD_TEST)
  include("BoostTest")
  enable_testing()
  add_subdirectory("test")
endif()
