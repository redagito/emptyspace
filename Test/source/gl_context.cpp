#include "gl_context.hpp"

GLFWwindow* GLTestContext()
{
    static GLFWwindow* const window = []()
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        if (!glfwInit())
        {
            return static_cast<GLFWwindow*>(nullptr);
        }

        auto* const created = glfwCreateWindow(64, 64, "emptyspace-test", nullptr, nullptr);
        if (created != nullptr)
        {
            glfwMakeContextCurrent(created);
            gladLoadGL();
        }

        return created;
    }();

    if (window != nullptr)
    {
        glfwMakeContextCurrent(window);
    }

    return window;
}
