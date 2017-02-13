Requirements
============
- ranges-v3 library - fork from user FloopCZ
- boost::python library
- OpenCV 3
- Python 3

Build
=====
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=~/.local ..
make -j5
```

Test
====
```
make test
```
