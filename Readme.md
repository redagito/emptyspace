# EmptySpace - Yet Another Space Game

![Screenshot 1](./doc/screenshot1.jpg)
![Screenshot 2](./doc/screenshot2.jpg)

Space flight Demo with OpenGL based deferred rendering.

## Project layout

| Directory     | Target            | Contents                          |
|---------------|-------------------|-----------------------------------|
| `EmptySpace/` | `EmptySpace.lib`  | The static library.               |
| `App/`        | `App.exe`         | The application.                  |
| `Test/`       | `Test.exe`        | Catch2 unit tests for the library. |

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

The mouse sets the rate the ship turns at. The engines brake towards the speed
they are commanded with.

## Requirements

* Conan 2.x
* CMake 3.25.x or newer
* Visual Studio 2022 or newer

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

### Visual Studio 2026

```
conan install . --build=missing -s build_type=Release -s compiler.version=195 -c tools.cmake.cmaketoolchain:generator="Visual Studio 18 2026" -c tools.microsoft.msbuild:vs_version=18 -c tools.cmake:cmake_program="C:/Progra~1/CMake/bin/cmake"
```

## Running

The assets are loaded from a `data` directory next to the executable.
```
cd build/App/Release
App.exe
```

## Tests

```
ctest --test-dir build -C Release --output-on-failure
```

The rendering tests need an OpenGL 4.6 context, they create a hidden window for
it.
