add_subdirectory("stream")

add_subdirectory("utility")

build_pyboost_test_module(pyboost_range_py_cpp pyboost_range.py.cpp)
add_python_test("test.python.pyboost_range" "pyboost_range.py")
