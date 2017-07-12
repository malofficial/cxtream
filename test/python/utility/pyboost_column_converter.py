#!/usr/bin/env python3

import numpy as np
import pyboost_column_converter_py_cpp as pycpp


def main():
    assert(pycpp.list1d() == [1, 2, 3])
    assert(pycpp.list2d() == [[1, 2, 3]] * 3)
    assert(pycpp.list3d() == [[[1, 2, 3]] * 3] * 3)
    assert(pycpp.columns() == {'Int': [1, 2], 'Double': [9., 10.]})


if __name__ == '__main__':
    main()
