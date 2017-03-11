if(NOT PYTHONINTERP_FOUND)
  find_package(PythonInterp REQUIRED)
endif()

execute_process(
  COMMAND ${PYTHON_EXECUTABLE} -c
    "import distutils.sysconfig; \
     print(distutils.sysconfig.get_config_var('EXT_SUFFIX'))"
  OUTPUT_VARIABLE PYTHON_SUFFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
