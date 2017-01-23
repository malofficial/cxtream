# Requirements
. ranges-v3 library - branch shared_view from user FloopCZ
. boost::python library
. python3

# Build
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=~/.local ..
make
make install
cd ..
```

# Test
```
python src/iterate_stream.py
```
