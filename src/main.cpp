#include <cstdio>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Error callback required for GLFW
void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error : %s\n", description);
}

// Converts rgb to uint32
uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 24) | (g << 16) | (b << 8) | 255;
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

    //  Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Error intializing GLEW, code: %d\n", err);
        glfwTerminate();
        return -1;
    }

    // Get OpenGL details
    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Set tranparent blueish black background for window
    glClearColor(0.0, 0.0, 0.05, 0.7);

    // Main game loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
