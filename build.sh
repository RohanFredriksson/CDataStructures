#!/bin/bash
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
make
ctest --output-on-failure