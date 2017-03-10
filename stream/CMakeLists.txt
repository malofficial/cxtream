cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project("mnist_stream" VERSION 0.1)

# ----------------------
# Find Cxtream Libraries
# ----------------------

find_package(Cxtream)

# --------------------
# Find Other Libraries
# --------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(OpenCV COMPONENTS core imgproc highgui REQUIRED)
include("FindPythonSuffix")

# --------------
# Compiler Flags
# --------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")

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
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE Cxtream::Core
  PRIVATE Cxtream::Python
  PRIVATE ${OpenCV_LIBRARIES}
)
