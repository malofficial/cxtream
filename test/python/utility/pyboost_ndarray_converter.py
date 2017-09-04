#!/usr/bin/env python3

import numpy as np
import pyboost_ndarray_converter_py_cpp as pycpp


def main():
    assert(np.array_equal(pycpp.py_empty_vec(), []))
    assert(pycpp.py_empty_vec().dtype == np.bool)
    assert(np.array_equal(pycpp.py_char_vec(), list("oops!")))
    assert(pycpp.py_char_vec().dtype == np.object)
    assert(np.array_equal(pycpp.py_int_vec(), [-1, 0, 1, 2, 3]))
    assert(pycpp.py_int_vec().dtype == np.int32)
    assert(np.array_equal(pycpp.py_double_vec(), [-1.5, -0.5, 0.5, 1.5, 2.5]))
    assert(pycpp.py_double_vec().dtype == np.double)

    obj_array = pycpp.py_object_vec()
    assert(isinstance(obj_array, np.ndarray))
    assert(len(obj_array) == 3)
    assert(obj_array.dtype == np.object)
    assert(obj_array[0].num == 1)
    assert(obj_array[1].num == 2)
    assert(obj_array[2].num == 3)


if __name__ == '__main__':
    main()
