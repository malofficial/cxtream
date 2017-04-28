add_subdirectory("core")

if(BUILD_TENSORFLOW)
  add_subdirectory("tensorflow")
endif()
