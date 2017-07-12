add_subdirectory("stream")

add_subdirectory("utility")

build_pyboost_test_module(pyboost_range_iterator_py_cpp pyboost_range_iterator.py.cpp)
add_python_test("test.python.pyboost_range_iterator" "pyboost_range_iterator.py")
