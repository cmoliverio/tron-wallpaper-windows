# tron-wallpaper-windows

This project is a from-scratch OpenGL application that will procedurally
generate light cycle traces just like in the `TRON: Legacy` movie intro.

## Quick Start

### Requirements

- Windows 10 or 11 (64-bit only)
- MSVC Compiler 19.38+
- Choco installer 
- BASH shell (I prefer Git BASH)
- Git v2.52+
- CMake v4.2.1+

--------------

### Prep 

1. From BASH:
```
$ choco install cmake
```

2. Install vcpkg and run 
```
$ git clone https://github.com/microsoft/vcpkg.git
$ cd vcpkg
$ ./bootstrap-vcpkg.sh
$ ./vcpkg install glfw3:x64-windows-static glm:x64-windows-static glad:x64-windows-static
```

3. Get the CMAKE_TOOLCHAIN_FILE from vcpkg
```
$ ./vcpkg.exe integrate install

# should see an output something like '-DCMAKE_TOOLCHAIN_FILE=<some_dir>'
# need this for later
```

4. Build and run
```
$ export CMAKE_TOOLCHAIN_FILE=<the_dir_that_vcpkg_says_to_use>
$ ./build.sh
$ ./tron_wallpaper.exe
```

Quick build and run:
```
cmake --build build --config Release && ./tron_wallpaper.exe

```