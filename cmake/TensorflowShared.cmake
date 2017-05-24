include(ExternalProject)

ExternalProject_Add(
  tensorflow_shared
  PREFIX third_party
  GIT_REPOSITORY http://github.com/tensorflow/tensorflow.git
  GIT_TAG v1.2.0-rc0
  TMP_DIR "/tmp"
  STAMP_DIR "third_party/tensorflow-stamp"
  DOWNLOAD_DIR "third_party/tensorflow"
  SOURCE_DIR "third_party/tensorflow"
  BUILD_IN_SOURCE 1
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND make -f tensorflow/contrib/makefile/Makefile clean
            COMMAND tensorflow/contrib/makefile/download_dependencies.sh > /dev/null
            COMMAND tensorflow/contrib/makefile/compile_linux_protobuf.sh > /dev/null
            COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_tensorflow.sh" .
            COMMAND ./build_tensorflow.sh > /dev/null
            COMMAND cp bazel-bin/tensorflow/libtensorflow_cc.so "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libcxtream_tensorflow_cc.so"
            COMMAND chmod u+rwx "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libcxtream_tensorflow_cc.so"
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
