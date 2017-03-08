# TODO add cxtream package
find_package(OpenCV COMPONENTS core imgproc highgui REQUIRED)

add_python_stream("mnist_stream" "mnist_stream_py.cpp"
  "PRIVATE ${OpenCV_INCLUDE_DIRS}"
  "${OpenCV_LIBRARIES}"
)
