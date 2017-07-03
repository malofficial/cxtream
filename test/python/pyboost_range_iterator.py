#!/usr/bin/env python3

import numpy as np
import pyboost_range_iterator_py_cpp as pycpp


def main():
    assert(list(pycpp.empty_iterator()) == [])
    assert(list(pycpp.fib_iterator())   == [1, 1, 2, 3, 5, 8])


if __name__ == '__main__':
    main()
