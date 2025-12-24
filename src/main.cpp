#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>

// #include <shader_utils.hpp>
#include "shader.hpp"

float vertices[] = {
    // position x y z,      colors
    0.5f, -0.5f, 0.0f,      1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f,      0.0f, 0.0f, 1.0f};

float second_vertices[] = {
    -1.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
    0.0f, 0.01f, 0.0f,     0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f
};

static void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // OpenGL 3.3 Core (well-supported on Windows)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(
        960, 720,
        "OpenGL Windows App",
        nullptr,
        nullptr);

    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";

    uint32_t VBO;
    uint32_t VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    // move trinagle data in there
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    uint32_t VBO2;
    uint32_t VAO2;

    glGenBuffers(1, &VBO2);
    glGenVertexArrays(1, &VAO2);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(second_vertices),
                 second_vertices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Shader normal_shader("vertex_shader.vert", "fragment_shader.frag");
    Shader shader_2("vertex_shader2.vert", "yellow_fragment_shader.frag");

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.08f, 0.10f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // float time = (float)glfwGetTime();
        // float green_value = (sin(time) / 2.0f) + 0.5f;
        // int vertex_color_location = glGetUniformLocation(yellow_shader_program,
        //                                                  "myColor");
        //
        shader_2.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        normal_shader.use();
        // glUseProgram(normal_shader.get());
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 3);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}