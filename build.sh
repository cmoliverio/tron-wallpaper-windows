#!/bin/sh

rm -rf build 

cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake .

cmake --build build --config Release