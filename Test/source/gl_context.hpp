#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// A hidden window with an OpenGL 4.6 core context, created once per process.
// Returns nullptr when no context could be created.
GLFWwindow* GLTestContext();
