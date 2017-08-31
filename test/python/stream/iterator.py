#!/usr/bin/env python3

import iterator_py_cpp as pycpp


def main():
    assert(list(pycpp.empty_iterator()) == [])

    empty_batch_iterator = list(pycpp.empty_batch_iterator())
    assert(len(empty_batch_iterator) == 1)
    assert(set(empty_batch_iterator[0].keys()) == {"Int", "Double"})
    assert(list(empty_batch_iterator[0]["Int"]) == [])
    assert(list(empty_batch_iterator[0]["Double"]) == [])

    number_stream = list(pycpp.number_iterator())
    assert(len(number_stream)) == 2
    assert(set(number_stream[0].keys()) == {"Int", "Double"})
    assert(set(number_stream[1].keys()) == {"Int", "Double"})
    assert(list(number_stream[0]["Int"]) == [3, 2])
    assert(list(number_stream[1]["Int"]) == [1, 4])
    assert(list(number_stream[0]["Double"]) == [5.])
    assert(list(number_stream[1]["Double"]) == [2.])


if __name__ == '__main__':
    main()
