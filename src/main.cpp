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

    return 0;
}
