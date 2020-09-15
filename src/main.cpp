#include <cstdio>
#include <GLFW/glfw3.h>

// Error callback required for GLFW
void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error : %s\n", description);
}

int main(int argc, char *argv[])
{
    glfwSetErrorCallback(error_callback);

    // Initialize GLFW
    if (!glfwInit())
    {
        return -1;
    }

    // Hints to make GLFW use OpenGL context version 3.3 or higher
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create Main Window
    GLFWwindow *window = glfwCreateWindow(640, 480, "Invaders", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    return 0;
}
