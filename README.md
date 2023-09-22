# EmptySpace - Yet Another Space Game

![Screenshot 1](./doc/screenshot1.jpg)
![Screenshot 2](./doc/screenshot2.jpg)

Space flight Demo with OpenGL based deferred rendering.

## Controls

```
 - W = accelerate forward
 - S = accelerate backward
 - A = accelerate to the left
 - D = accelerate to the right
 - Space = accelerate up
 - LCtrl = accelerate down
 - LShift = boost by factor 40
 - Q = roll left
 - E = roll right
 - R = stop acceleration
 - Mouse = define direction to accelerate
```

## Requirements

Development is done with
* Conan 2.x
* CMake 3.27.x
* Visual Studio 2022 Community Edition

External dependencies
* GLFW
* GLAD
* PhysX
* stb_image
* glm
* assimp

## Building

Conan install for debug and release build types
This step may take a while due to PhysX
```
conan install . --build=missing --settings=build_type=Debug && conan install . --build=missing --settings=build_type=Release
```

CMake project generation
```
cmake --preset conan-default
```
