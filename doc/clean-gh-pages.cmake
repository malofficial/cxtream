# remove all files except for the hidden ones
file(GLOB _gh_files "[^.]*")
if(_gh_files)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E remove ${_gh_files})
  foreach(_file ${_gh_files})
    execute_process(COMMAND "${CMAKE_COMMAND}" -E remove_directory ${_file})
  endforeach()
endif()
