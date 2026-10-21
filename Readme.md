# EmptySpace - Yet Another Space Game

![Screenshot 1](./doc/screenshot1.jpg)
![Screenshot 2](./doc/screenshot2.jpg)

Space flight Demo with OpenGL based deferred rendering.

## Project layout

| Directory     | Target            | Contents                                                    |
|---------------|-------------------|-------------------------------------------------------------|
| `EmptySpace/` | `EmptySpace.lib`  | The static library. All rendering, scene and physics logic. |
| `App/`        | `App.exe`         | The application. Window, input and the frame loop.          |
| `Test/`       | `Test.exe`        | Catch2 unit tests for the library.                          |

The library is compiled once, the application and the tests link against it.
The root `CMakeLists.txt` is an umbrella that includes all three.

```
EmptySpace/include   public headers
EmptySpace/source    implementation
App/source            main.cpp
Test/source           unit tests
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

The ship is commanded in its own frame. The mouse sets the rate the ship turns
at, not how far it turns, and the engines brake towards the speed they are
commanded with.

## Requirements

* Conan 2.x
* CMake 3.25.x or newer
* Visual Studio 2022 or newer

External dependencies: GLFW, GLAD, PhysX, stb_image, glm, assimp, Catch2

## Building

### Linux

Debug
```
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build build/Debug
```

Release
```
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build build/Release
```

### Windows

```
conan install . --build=missing -s build_type=Debug && conan install . --build=missing -s build_type=Release
cmake --preset conan-default
cmake --build build --config Release
```

Produces
```
build/EmptySpace/Release/EmptySpace.lib
build/App/Release/App.exe
build/Test/Release/Test.exe
```

### Visual Studio 2026

```
conan install . --build=missing -s build_type=Release -s compiler.version=195 -c tools.cmake.cmaketoolchain:generator="Visual Studio 18 2026" -c tools.microsoft.msbuild:vs_version=18 -c tools.cmake:cmake_program="C:/Progra~1/CMake/bin/cmake"
```

## Running

The application loads its assets from a `data` directory relative to the working
directory, the build copies it next to the executable.

```
cd build/App/Release
App.exe
```

## Running the tests

```
ctest --test-dir build -C Release --output-on-failure
```

or
```
build/Test/Release/Test.exe
```

The rendering tests need an OpenGL 4.6 context, they create a hidden window for
it.
