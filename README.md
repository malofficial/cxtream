# cxtream
C++17 data pipeline with Python bindings.

## Development Status
![Build Status](https://gitlab.com/Cognexa/cxtream/badges/master/build.svg)
This project is under heavy development. The API is continuously changing without regard to backward compatibility.

## Requirements
The cxtream core is a header-only C++ library with no requirements. However,
if you want to use the Python bindings including the OpenCV support,
the following dependencies have to be installed:

__Arch Linux__
```
pacman -S base-devel cmake opencv boost python python-numpy
```

__Ubuntu 16.10+__
```
apt install build-essential cmake libopencv-dev libboost-python-dev libboost-test-dev python3-dev python3-numpy
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
make install
```

## License

The code is distributed under the MIT license. See the [LICENSE](LICENSE.txt) file for the full license agreement.
