#include <cstdio>
#include <cstdint>
#include <limits>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

bool game_running = false;
int move_dir = 0;
bool fire_pressed = 0;

#define GL_ERROR_CASE(glerror) \
    case glerror:              \
        snprintf(error, sizeof(error), "%s", #glerror)

//  Debug OpenGL
inline void gl_debug(const char *file, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        char error[128];

        switch (err)
        {
            GL_ERROR_CASE(GL_INVALID_ENUM);
            break;
            GL_ERROR_CASE(GL_INVALID_VALUE);
            break;
            GL_ERROR_CASE(GL_INVALID_OPERATION);
            break;
            GL_ERROR_CASE(GL_INVALID_FRAMEBUFFER_OPERATION);
            break;
            GL_ERROR_CASE(GL_OUT_OF_MEMORY);
            break;
        default:
            snprintf(error, sizeof(error), "%s", "UNKNOWN_ERROR");
            break;
        }

        fprintf(stderr, "%s - %s: %d\n", error, file, line);
    }
}

#undef GL_ERROR_CASE

// Validate shader for errors
void validate_shader(GLuint shader, const char *file = 0)
{
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);

    if (length > 0)
    {
        printf("Shader %d(%s) compile error: %s\n", shader, (file ? file : ""), buffer);
    }
}

// Validate program for errors
bool validate_program(GLuint program)
{
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

    if (length > 0)
    {
        printf("Program %d link error: %s\n", program, buffer);
        return false;
    }

    return true;
}

// Error callback required for GLFW
void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error : %s\n", description);
}

// Handles keyboard input
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            game_running = false;
        break;
    case GLFW_KEY_RIGHT:
        if (action == GLFW_PRESS)
            move_dir += 1;
        else if (action == GLFW_RELEASE)
            move_dir -= 1;
        break;
    case GLFW_KEY_LEFT:
        if (action == GLFW_PRESS)
            move_dir -= 1;
        else if (action == GLFW_RELEASE)
            move_dir += 1;
        break;
    case GLFW_KEY_SPACE:
        if (action == GLFW_RELEASE)
            fire_pressed = true;
        break;
    default:
        break;
    }
}

// Shift ranges
uint32_t xorshift32(uint32_t *rng)
{
    uint32_t x = *rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *rng = x;
    return x;
}

// Generate random rumber
double random(uint32_t *rng)
{
    return (double)xorshift32(rng) / std::numeric_limits<uint32_t>::max();
}

// Converts rgb to uint32
uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 24) | (g << 16) | (b << 8) | 255;
}

struct Buffer
{
    size_t width, height;
    uint32_t *data;
};

struct Sprite
{
    size_t width, height;
    uint8_t *data;
};

struct Alien
{
    size_t x, y;
    uint8_t type;
};

struct Bullet
{
    size_t x, y;
    int dir;
};

struct Player
{
    size_t x, y;
    size_t life;
};

// Main game struct
#define GAME_MAX_BULLETS 128
struct Game
{
    size_t width, height;
    size_t num_aliens;
    size_t num_bullets;
    Alien *aliens;
    Player player;
    Bullet bullets[GAME_MAX_BULLETS];
};

// Sprite amimation with frames
struct SpriteAnimation
{
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    Sprite **frames;
};

// Different Alien types
enum AlienType : uint8_t
{
    ALIEN_DEAD = 0,
    ALIEN_TYPE_A = 1,
    ALIEN_TYPE_B = 2,
    ALIEN_TYPE_C = 3
};

// Clear buffer
void buffer_clear(Buffer *buffer, uint32_t color)
{
    for (size_t i = 0; i < buffer->width * buffer->height; ++i)
    {
        buffer->data[i] = color;
    }
}

// Check if sprite boxes are overlapping
bool sprite_overlap_check(
    const Sprite &sp_a, size_t x_a, size_t y_a,
    const Sprite &sp_b, size_t x_b, size_t y_b)
{
    if (x_a < x_b + sp_b.width && x_a + sp_a.width > x_b &&
        y_a < y_b + sp_b.height && y_a + sp_a.height > y_b)
    {
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    // Buffer size
    const size_t buffer_width = 224;
    const size_t buffer_height = 256;

    glfwSetErrorCallback(error_callback);

    // Initialize GLFW
    if (!glfwInit())
        return -1;

    // Hints to make GLFW use OpenGL context version 3.3 or higher
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create Main Window
    GLFWwindow *window = glfwCreateWindow(buffer_width, buffer_height, "Invaders", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    // Handle kyboard inputs
    glfwSetKeyCallback(window, key_callback);
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

    gl_debug(__FILE__, __LINE__);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    glfwSwapInterval(1);

    // Set tranparent blueish black background for window
    glClearColor(0.0, 0.0, 0.05, 0.7);

    // Create graphics buffer
    uint32_t clear_color = rgb_to_uint32(32, 128, 200);
    Buffer buffer;
    buffer.width = buffer.width;
    buffer.height = buffer.height;
    buffer.data = new uint32_t[buffer.width * buffer.height];

    buffer_clear(&buffer, clear_color);

    // Create texture for presenting buffer to OpenGL
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create vao for generating fullscreen triangle
    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);

    // OpenGL vertex shader
    const char *vertex_shader =
        "\n"
        "#version 330\n"
        "\n"
        "noperspective out vec2 TexCoord;\n"
        "\n"
        "void main(void){\n"
        "\n"
        "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
        "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
        "    \n"
        "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
        "}\n";

    // OpenGL fragment shader
    const char *fragment_shader =
        "\n"
        "#version 330\n"
        "\n"
        "uniform sampler2D buffer;\n"
        "noperspective in vec2 TexCoord;\n"
        "\n"
        "out vec3 outColor;\n"
        "\n"
        "void main(void){\n"
        "    outColor = texture(buffer, TexCoord).rgb;\n"
        "}\n";

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
