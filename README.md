# cxtream
C++17 data pipeline with Python bindings.

## Development Status

- [![Build Status](https://gitlab.com/Cognexa/cxtream/badges/master/build.svg)](https://gitlab.com/Cognexa/cxtream/builds/)
- [![Development Status](https://img.shields.io/badge/status-CX%20PoC-yellow.svg?style=flat)]()
- [![Master Developer](https://img.shields.io/badge/master-Filip%20Matzner-lightgrey.svg?style=flat)]()

**This project is under heavy development. The API is continuously changing without regard to backward compatibility.**

## Requirements
The cxtream core is a header-only C++ library with dependencies
to header-only parts of [Boost C++ Libraries](http://www.boost.org/).

To install Boost, run the following command:

__Arch Linux__
```
pacman -S boost
```

__Ubuntu 16.10+__
```
apt install libboost-dev
```

However,
if you plan to use the Python bindings including the OpenCV support,
there are a few more dependencies that have to be installed.

The following command may be used to install all the requirements automatically.

__Arch Linux__
```
pacman -S base-devel cmake opencv boost python python-numpy
```

__Ubuntu 16.10+__
```
apt install build-essential cmake libopencv-dev libboost-dev libboost-python-dev libboost-test-dev python3-dev python3-numpy
```

## Installation

__Build__

```
mkdir build && cd build
cmake ..
make -j5
```

__Test__
```
make test
```

__Install__
```
sudo make install
```

## Example

Please refer to [Cognexa/mnist-example](https://gitlab.com/Cognexa/mnist-example) repository for a usage example.
