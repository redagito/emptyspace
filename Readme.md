# EmptySpace - Yet Another Space Game

![Screenshot 1](./doc/screenshot1.jpg)
![Screenshot 2](./doc/screenshot2.jpg)

Space flight Demo with OpenGL based deferred rendering.

## Project layout

| Directory    | Target                | Contents                                                        |
|--------------|-----------------------|-----------------------------------------------------------------|
| `EmptySpace/`| `EmptySpace.lib`      | The static library. All rendering, scene and physics logic.     |
| `App/`       | `App.exe`             | The application. Window, input and the frame loop only.         |
| `Test/`      | `Test.exe`            | Catch2 unit tests for the library.                              |

The library is compiled once; the application and the tests link against it.
Each directory is a self-contained CMake project and can be configured on its
own, the root `CMakeLists.txt` is only an umbrella that includes all three.

```
EmptySpace/
  include/       public headers
  source/        implementation
App/
  source/        main.cpp
Test/
  source/        unit tests
```

## Controls

```
 - W = accelerate forward
 - S = accelerate backward
 - A = accelerate to the left
 - D = accelerate to the right
 - Space = accelerate up
 - LCtrl = accelerate down
 - LShift = boost by factor 4
 - Q = roll left
 - E = roll right
 - R = stop
 - Mouse = steer
 - F1 = transition effect
 - Esc = quit
```

The ship is commanded in its own frame: W moves it along its nose and the mouse
steers it. The mouse does not turn the ship by the distance it moved, it sets
the rate the ship turns at, so the ship keeps turning while the mouse moves and
holds its attitude when the mouse stops. The engines brake towards the speed
they are commanded with, so the ship comes to a halt when the keys are
released.

## Requirements

Development is done with
* Conan 2.x
* CMake 3.25.x or newer
* Visual Studio 2022 or newer

External dependencies
* GLFW
* GLAD
* PhysX
* stb_image
* glm
* assimp
* Catch2

## Building

The dependencies are resolved for the compiler of your Conan profile. When
Conan has a binary for it the install is quick, otherwise the packages are
built from source, which takes a long time because of the size of PhysX.

### Linux

Conan install for debug or release build type
```
conan install . --build=missing -s build_type=Debug
```

or release build type
```
conan install . --build=missing -s build_type=Release
```

CMake project generation for debug
```
cmake --preset conan-debug
```

or relase build
```
cmake --preset conan-release
```

Build the project in debug config with
```
cmake --build build/Debug
```

or in release config with
```
cmake --build build/Release
```

### Windows

Conan install for debug and release build types for multi generator
```
conan install . --build=missing -s build_type=Debug && conan install . --build=missing -s build_type=Release
```

CMake project generation
```
cmake --preset conan-default
```

Open up the solution file in the build folder and build the project

The build produces
```
build/EmptySpace/Release/EmptySpace.lib
build/App/Release/App.exe
build/Test/Release/Test.exe
```

### When a dependency is built from source

Conan builds a package it has no binary for with the CMake it ships, and that
CMake does not know every generator. For Visual Studio 2026 the generator and
a recent CMake have to be named explicitly
```
conan install . --build=missing -s build_type=Release -c tools.cmake.cmaketoolchain:generator="Visual Studio 18 2026" -c tools.microsoft.msbuild:vs_version=18 -c tools.cmake:cmake_program="C:/Progra~1/CMake/bin/cmake"
```

The compiler of the profile has to match the one the packages are built for,
so add its version to the install
```
conan install . --build=missing -s build_type=Release -s compiler.version=195
```

### Building a single project

Each project can be configured on its own, it pulls in the library it needs.
The toolchain file Conan generated has to be given to it, and it has to be an
absolute path
```
cmake -S App -B build/App -DCMAKE_TOOLCHAIN_FILE=<path to build/generators/conan_toolchain.cmake>
```

## Running

The application loads its assets from a `data` directory relative to the
working directory. The build copies that directory next to the executable, so
run the executable from the directory it was built into.

```
cd build/App/Release
App.exe
```

## Running the tests

The unit tests are registered with CTest
```
ctest --test-dir build -C Release --output-on-failure
```

or run the test executable directly
```
build/Test/Release/Test.exe
```

The tests that render need an OpenGL 4.6 context. They create a hidden window
for it, so a display has to be available, but nothing is shown and the mouse
is not captured. The rendering tests read the render targets back and compare
them against what the scene should produce.
