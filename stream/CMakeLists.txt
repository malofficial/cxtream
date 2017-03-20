cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project("mnist_stream" VERSION 0.1)

# --------------
# Find Libraries
# --------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# OpenCV
find_package(OpenCV COMPONENTS core imgproc highgui REQUIRED)
# Boost
find_package(Boost COMPONENTS iostreams REQUIRED)
# cxtream
find_package(Cxtream REQUIRED)
# yaml-cpp TODO this package contains invalid CMake Config. Fix upstream.
# find_package(yaml-cpp REQUIRED)
# python .so file suffix
include("FindPython3Suffix")

# --------------
# Compiler Flags
# --------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -pedantic -pipe")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -fno-inline -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Ofast")

# ----------------------
# Shorter Error Messages
# ----------------------

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # GCC truncates multiline errors with -Wfatal-errors
  # using -fmax-erorrs instead
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmax-errors=2")
endif()

# -------------------
# Build Python Stream
# -------------------

add_library(
  ${PROJECT_NAME} SHARED
  "${PROJECT_NAME}_py.cpp"
)

set_target_properties(
  ${PROJECT_NAME} PROPERTIES
  PREFIX ""
  OUTPUT_NAME ${PROJECT_NAME}
  SUFFIX ${PYTHON_SUFFIX}
  VERSION ${PROJECT_VERSION}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${OpenCV_INCLUDE_DIRS}
  PRIVATE ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE yaml-cpp
  PRIVATE Cxtream::Core
  PRIVATE Cxtream::Python
  PRIVATE ${OpenCV_LIBRARIES}
  PRIVATE ${Boost_LIBRARIES}
)
