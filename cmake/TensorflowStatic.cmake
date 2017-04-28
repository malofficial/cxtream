include(ExternalProject)

ExternalProject_Add(
  tensorflow_static
  PREFIX third_party
  GIT_REPOSITORY http://github.com/tensorflow/tensorflow.git
  # GIT_TAG v1.1.0-rc1
  GIT_TAG 163da25caf2b63ba61fe94821de65c5362249691
  TMP_DIR "/tmp"
  STAMP_DIR "third_party/tensorflow-stamp"
  DOWNLOAD_DIR "third_party/tensorflow"
  SOURCE_DIR "third_party/tensorflow"
  BUILD_IN_SOURCE 1
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND make -f tensorflow/contrib/makefile/Makefile clean
            COMMAND tensorflow/contrib/makefile/download_dependencies.sh > /dev/null
            COMMAND tensorflow/contrib/makefile/build_all_linux.sh > /dev/null
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
