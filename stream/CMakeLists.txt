add_python_stream("mnist_stream" "mnist_stream_py.cpp"
  "PRIVATE ${OpenCV_INCLUDE_DIRS}"
  "${OpenCV_LIBRARIES}"
)
