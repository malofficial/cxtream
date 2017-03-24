cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(
  "cxtream"
  VERSION 0.1
)
set(PROJECT_DESCRIPTION "C++17 data pipeline with Python bindings.")

# -------
# Options
# -------

option(BUILD_TEST "Build test binaries" ON)
option(BUILD_PYTHON "Build C++ <-> Python converters" ON)
option(BUILD_PYTHON_OPENCV "Build C++ <-> Python OpenCV converters (requires BUILD_PYTHON)" ON)

# -------------
# CMake Options
# -------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

# -------------------------------------
# Dump CMake Options Into a Header File
# -------------------------------------

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_config.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/cxtream/build_config.hpp"
)

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

# ---------------------
# Find Common Libraries
# ---------------------

if(BUILD_PYTHON)
  find_package(PythonInterp 3 REQUIRED)
  find_package(PythonLibs 3 REQUIRED)
  find_package(BoostPython3 REQUIRED)

  if(BUILD_PYTHON_OPENCV)
    find_package(PythonNumPy REQUIRED)
    find_package(OpenCV COMPONENTS core REQUIRED)
  endif()
endif()


# ------------------
# Build cxtream core
# ------------------

add_library(cxtream_core INTERFACE)
set_target_properties(
  cxtream_core PROPERTIES EXPORT_NAME Core
)
target_compile_options(
  cxtream_core INTERFACE
  "-std=c++1z"
)
target_link_libraries(
  cxtream_core INTERFACE
  "-lpthread"
  "-lstdc++fs"
)
target_include_directories(
  cxtream_core INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> # build_config.hpp is here
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/cxtream/range-v3>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/cxtream/range-v3>
)

# ---------------------
# Build cxtream::python
# ---------------------

if(BUILD_PYTHON)

  # -------------------------
  # Build cxtream::python .so
  # -------------------------

  add_library(
    cxtream_python SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/src/python/pyboost_initialize.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/python/utility/pyboost_fs_path_converter.cpp"
  )
  set_target_properties(
    cxtream_python PROPERTIES EXPORT_NAME Python
  )
  target_include_directories(
    cxtream_python
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${PYTHON_INCLUDE_DIRS}
  )
  target_link_libraries(
    cxtream_python
    cxtream_core
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    stdc++fs
  )
  set_target_properties(
    cxtream_python
    PROPERTIES VERSION ${PROJECT_VERSION}
  )

  # -----------------------------------------------------
  # Build cxtream::python::utility::pyboost_cv3_converter
  # -----------------------------------------------------

  if(BUILD_PYTHON_OPENCV)
    target_sources(
      cxtream_python
      PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/python/utility/pyboost_cv2_converter.cpp"
      PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/python/utility/pyboost_cv3_converter.cpp"
    )
    target_include_directories(
      cxtream_python
      PUBLIC ${PYTHON_NUMPY_INCLUDE_DIRS}
      PUBLIC ${OpenCV_INCLUDE_DIRS}
    )
    target_link_libraries(
      cxtream_python
      ${OpenCV_LIBRARIES}
    )
  endif()

endif()

# -----
# Tests
# -----

if(BUILD_TEST)
  find_package(Boost COMPONENTS unit_test_framework REQUIRED)
  enable_testing()
  include("AddBoostTest")
  add_subdirectory("test")
endif()

# -------
# Install
# -------

# install built-in range-v3 library
install(
  DIRECTORY include/cxtream/range-v3/
  DESTINATION include/cxtream/range-v3
)

# install core header files
install(
  DIRECTORY include/cxtream/core/
  DESTINATION include/cxtream/core
)
install(
  FILES include/cxtream/core.hpp
  DESTINATION include/cxtream
)

# install generated files (e.g., build_config.hpp)
install(
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/"
  DESTINATION include
)

# install core target
install(
  TARGETS cxtream_core
  EXPORT CxtreamCoreTargets
)

# install python library
if(BUILD_PYTHON)
  install(
    DIRECTORY include/cxtream/python/
    DESTINATION include/cxtream/python
  )
  install(
    FILES include/cxtream/python.hpp
    DESTINATION include/cxtream
  )
  install(
    TARGETS cxtream_python
    EXPORT CxtreamPythonTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
endif()

# ---------
# Uninstall
# ---------

add_custom_target(
  uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Uninstall.cmake
)

# ------------------------------
# Cmake Config and Version Files
# ------------------------------

include(CMakePackageConfigHelpers)
set(CMAKECFG_INSTALL_DIR lib/cmake/Cxtream)

# configure package files
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/CxtreamConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CxtreamConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CxtreamConfig.cmake"
  INSTALL_DESTINATION "${CMAKECFG_INSTALL_DIR}"
  NO_SET_AND_CHECK_MACRO # Cxtream only uses interface libraries
  NO_CHECK_REQUIRED_COMPONENTS_MACRO # Cxtream does not have components
)

# install the core targets file
install(
  EXPORT CxtreamCoreTargets
  FILE CxtreamCoreTargets.cmake
  NAMESPACE Cxtream::
  DESTINATION "${CMAKECFG_INSTALL_DIR}"
)
# export the targets also for local use
export(
  TARGETS cxtream_core
  FILE CxtreamCoreTargets.cmake
  NAMESPACE Cxtream::
)

# install the python targets file
if(BUILD_PYTHON)
  install(
    EXPORT CxtreamPythonTargets
    FILE CxtreamPythonTargets.cmake
    NAMESPACE Cxtream::
    DESTINATION "${CMAKECFG_INSTALL_DIR}"
  )
  # export the targets also for local use
  export(
    TARGETS cxtream_python
    FILE CxtreamPythonTargets.cmake
    NAMESPACE Cxtream::
  )
endif()

# install the package files
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CxtreamConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CxtreamConfigVersion.cmake
  DESTINATION ${CMAKECFG_INSTALL_DIR}
)
