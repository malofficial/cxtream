set(Python_ADDITIONAL_VERSIONS "3.X")
find_package(PythonInterp REQUIRED)

execute_process(
  COMMAND ${PYTHON_EXECUTABLE} -c
    "import distutils.sysconfig; \
     print(distutils.sysconfig.get_config_var('EXT_SUFFIX'))"
  OUTPUT_VARIABLE PYTHON_SUFFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
