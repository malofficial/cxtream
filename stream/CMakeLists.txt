add_python_stream("simple_stream" "simple_stream.hpp"
  "PRIVATE ${OpenCV_INCLUDE_DIRS}"
  "${OpenCV_LIBRARIES}"
)

add_python_stream("column_stream" "column_stream.hpp"
  "PRIVATE ${OpenCV_INCLUDE_DIRS}"
  "${OpenCV_LIBRARIES}"
)
