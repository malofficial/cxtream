cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

configure_file(Doxyfile.in Doxyfile @ONLY)
add_custom_target(
  doc
  COMMAND ${CMAKE_COMMAND} -E remove_directory html
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
  COMMENT "Generating API documentation with Doxygen"
  VERBATIM
)
