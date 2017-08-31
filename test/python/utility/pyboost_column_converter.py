#!/usr/bin/env python3

import numpy as np
import pyboost_column_converter_py_cpp as pycpp


def main():
    assert(np.array_equal(pycpp.py_vector1d_empty(), []))

    # test whole range
    assert(np.array_equal(pycpp.py_vector1d(), [1, 2, 3]))
    assert(np.array_equal(pycpp.py_vector2d(), [[1, 2, 3]] * 3))
    assert(np.array_equal(pycpp.py_vector3d(), [[[1, 2, 3]] * 3] * 3))

    # test slicing
    assert(np.array_equal(pycpp.py_vector1d()[ 0: 3], [1, 2, 3]))
    assert(np.array_equal(pycpp.py_vector1d()[ 1: 3], [2, 3]))
    assert(np.array_equal(pycpp.py_vector1d()[ 2: 3], [3]))
    assert(np.array_equal(pycpp.py_vector1d()[ 3: 3], []))
    assert(np.array_equal(pycpp.py_vector1d()[-1: 7], [3]))
    assert(np.array_equal(pycpp.py_vector1d()[-2:-1], [2]))

    # It is not necessary to test the conversion more in here, since it is
    # already covered by pyboost_range or python's list.

    columns = pycpp.columns()
    assert(set(columns.keys()) == {"Int", "Double"})
    assert(list(columns["Int"]) == [1, 2])
    assert(list(columns["Double"]) == [9., 10.])


if __name__ == '__main__':
    main()
