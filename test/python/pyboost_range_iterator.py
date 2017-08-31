#!/usr/bin/env python3

import numpy as np
import pyboost_range_iterator_py_cpp as pycpp


def main():
    assert(list(pycpp.empty_iterator()) == [])
    assert(list(pycpp.fib_iterator())   == [1, 1, 2, 3, 5, 8])
    assert(list(pycpp.view_iterator())  == [1, 1, 2, 3, 5, 8])

    # test indexing
    cpp_data = pycpp.fib_iterator()
    list_data = list(pycpp.fib_iterator())
    for i in range(-6, 5):
        assert(cpp_data[i] == list_data[i])

    # test slicing
    for i in range(-15, 15):
        for j in range(-15, 15):
            assert(list(cpp_data[i:j]) == list_data[i:j])
            assert(list(cpp_data[i:]) == list_data[i:])
            assert(list(cpp_data[:j]) == list_data[:j])


if __name__ == '__main__':
    main()
