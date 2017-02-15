function(add_boost_test EXECUTABLE_FILE_NAME SOURCE_FILE_NAME INCLUDES LIBRARIES)
  add_executable(
    ${EXECUTABLE_FILE_NAME}
    ${SOURCE_FILE_NAME}
  )

  target_link_libraries(
    ${EXECUTABLE_FILE_NAME} 
    ${DEPENDENCY_LIB}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${LIBRARIES}
  )

  target_include_directories(
    ${EXECUTABLE_FILE_NAME}
    PRIVATE ${Boost_INCLUDE_DIRS}
    ${INCLUDES}
  )

  add_test(
    NAME "${EXECUTABLE_FILE_NAME}" 
    COMMAND ${EXECUTABLE_FILE_NAME}
    --catch_system_error=yes
  )
endfunction()
