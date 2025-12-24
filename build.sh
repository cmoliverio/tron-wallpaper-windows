#!/bin/sh

rm -rf build 

# make sure that cmake is installed
command -v cmake >/dev/null 2>&1 || {
    echo "cmake not found"
    exit 1
}

# make sure CMAKE_TOOLCHAIN_FILE is set
if [[ -z "$CMAKE_TOOLCHAIN_FILE" ]]; then
    echo "CMAKE_TOOLCHAIN_FILE not set"
    exit 1
fi
# make sure it points to a real file
if [[ ! -f "$CMAKE_TOOLCHAIN_FILE" ]]; then
    echo "Invalid CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
    exit 1
fi

# run cmake using the env var
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
  .

cmake -B build

cmake --build build --config Release
