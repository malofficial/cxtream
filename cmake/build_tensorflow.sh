#!/bin/sh
set -e
VERSION="1.1.0-rc1"

# download ternsorflow as an archive
# TF_FOLDER="tensorflow-v${VERSION}"
# if [ ! -d $TF_FOLDER ]; then
#     wget "https://github.com/tensorflow/tensorflow/archive/v${VERSION}.tar.gz"
#     tar -xzf "v${VERSION}.tar.gz"
#     rm "v${VERSION}.tar.gz"
# fi

# clone tensorflow from git
# TF_FOLDER="tensorflow"
# if [ ! -d $TF_FOLDER ]; then
#     git clone "https://github.com/tensorflow/tensorflow.git"
# fi

# assume that tensorflow is already built
TF_FOLDER="."

# configuration variables
export CC_OPT_FLAGS="-march=native"
export TF_NEED_GCP=0
export TF_NEED_HDFS=0
export TF_NEED_OPENCL=0
export TF_NEED_JEMALLOC=1
export TF_NEED_VERBS=0
export TF_NEED_MKL=0
export TF_ENABLE_XLA=1
export TF_CUDA_CLANG=0
export PYTHON_BIN_PATH=/usr/bin/python
export PYTHON_LIB_PATH=$($PYTHON_BIN_PATH -c 'import site; print(site.getsitepackages()[0])')

# configuration variables
if [ -e /opt/cuda ]; then
    echo "CUDA support enabled"
    cuda_config_opts="--config=cuda"
    export TF_NEED_CUDA=1
    export TF_CUDA_COMPUTE_CAPABILITIES="3.5,5.2"
    export CUDA_TOOLKIT_PATH=/opt/cuda
    export CUDNN_INSTALL_PATH=/opt/cuda
    export TF_CUDA_VERSION=$($CUDA_TOOLKIT_PATH/bin/nvcc --version | sed -n 's/^.*release \(.*\),.*/\1/p')
    export TF_CUDNN_VERSION=$(sed -n 's/^#define CUDNN_MAJOR\s*\(.*\).*/\1/p' $CUDNN_INSTALL_PATH/include/cudnn.h)
    export GCC_HOST_COMPILER_PATH=/usr/bin/gcc-5
else
    echo "CUDA support disabled"
    cuda_config_opts=""
    export TF_NEED_CUDA=0
fi

# configure and build
cd "$TF_FOLDER"
./configure
bazel build -c opt $cuda_config_opts --copt=${CC_OPT_FLAGS} tensorflow:libtensorflow_cc.so
bazel shutdown
