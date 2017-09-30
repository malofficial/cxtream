Installation {#installation}
============

Requirements
------------
---

Officially supported systems are Ubuntu 16.10+ and Arch Linux, although __cxtream__ should
work on any recent enough system. The __cxtream__ core is a pure C++ library with a
single dependency to [Boost C++ Libraries](http://www.boost.org/)
(Boost 1.61+ is required).

If you want to use __cxtream core__ only, install Boost via one of the following commands:

```
# Arch Linux
pacman -S boost

# Ubuntu 16.10+
apt install libboost-all-dev
```

If you plan to use also the Python bindings with OpenCV support,
use one of the following commands to install all the additional dependencies.

```
# Arch Linux
pacman -S base-devel cmake opencv boost python python-numpy

# Ubuntu 16.10+
apt install build-essential cmake libopencv-dev libboost-all-dev python3-dev python3-numpy
```

If you plan to use [TensorFlow C++ API](https://www.tensorflow.org/api_guides/cc/guide),
please install the TensorFlow C++ library into your system using
[tensorflow_cc](https://github.com/FloopCZ/tensorflow_cc) project.

Download
--------
---

The complete source code can be downloaded from our official GitHub
[repository](https://github.com/Cognexa/cxtream) using the following commands:

```
git clone --recursive https://github.com/Cognexa/cxtream.git
cd cxtream
```

Build & Install
---------------
---

Use the following for system-wide installation:

```
mkdir build && cd build
cmake ..
make -j5
make test
sudo make install
```

Or use the following for userspacee installation:

```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=~/.local ..
make -j5
make test
make install
```

For userspace installation, don't forget to set the appropriate
environmental variables, e.g., add the following to your `.bashrc` / `.zshrc`:
```
# register ~/.local system hierarchy
export PATH="${HOME}/.local/bin:$PATH"
export LIBRARY_PATH="${HOME}/.local/lib:$LIBRARY_PATH"
export LD_LIBRARY_PATH="${HOME}/.local/lib:$LD_LIBRARY_PATH"
export CPLUS_INCLUDE_PATH="${HOME}/.local/include:$CPLUS_INCLUDE_PATH"
```
