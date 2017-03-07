function(add_python_stream STREAM_NAME SOURCE_FILE INCLUDES LIBRARIES)

  # -------------
  # Python Stream
  # -------------

  if(BUILD_PYTHON_LIB)

      # find python packages
      set(Python_ADDITIONAL_VERSIONS "3.X")
      find_package(Boost COMPONENTS python3 REQUIRED)
      find_package(PythonLibs REQUIRED)
      find_package(PythonInterp REQUIRED)
      include("FindPythonSuffix")

      # build stream dynamic library
      set(WORKDIR "${CMAKE_BINARY_DIR}/python/${STREAM_NAME}")
      file(
        GLOB_RECURSE pyboost_module_sources
        "${CMAKE_SOURCE_DIR}/src/pyboost_module/*.cpp"
        "${SOURCE_FILE}"
      )
      add_library(
        ${STREAM_NAME}_python_lib SHARED
        ${pyboost_module_sources}
      )
      target_include_directories(
        ${STREAM_NAME}_python_lib
        PRIVATE ${Boost_INCLUDE_DIRS}
        PRIVATE ${PYTHON_INCLUDE_DIRS}
        ${INCLUDES}
      )
      set_target_properties(
        ${STREAM_NAME}_python_lib PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${STREAM_NAME}
        SUFFIX ${PYTHON_SUFFIX}
        LIBRARY_OUTPUT_DIRECTORY "${WORKDIR}/lib"
      )
      target_link_libraries(
        ${STREAM_NAME}_python_lib
        stdc++fs
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        ${LIBRARIES}
      )

      # ----------------------
      # Python wheel installer
      # ----------------------

      if(BUILD_PYTHON_WHEEL)

        configure_file(
          "${CMAKE_SOURCE_DIR}/cmake/setup.py.in"
          "${WORKDIR}/setup.py"
        )
        configure_file(
          "${CMAKE_SOURCE_DIR}/cmake/setup.cfg.in"
          "${WORKDIR}/setup.cfg"
        )
        add_custom_target(
          ${STREAM_NAME}_python_wheel ALL
          COMMAND "${PYTHON_EXECUTABLE}" "setup.py" "bdist_wheel"
          WORKING_DIRECTORY "${WORKDIR}"
          DEPENDS ${STREAM_NAME}_python_lib
        )

      endif()
  endif()
endfunction()
