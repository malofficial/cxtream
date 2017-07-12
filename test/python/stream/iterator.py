#!/usr/bin/env python3

import numpy as np
import iterator_py_cpp as pycpp


def main():
    assert(list(pycpp.empty_iterator()) == [])
    assert(list(pycpp.empty_batch_iterator()) == [{"Int": [], "Double": []}])
    assert(list(pycpp.number_iterator()) == [{"Int": [3, 2], "Double": [5.]},
                                             {"Int": [1, 4], "Double": [2.]}])


if __name__ == '__main__':
    main()
