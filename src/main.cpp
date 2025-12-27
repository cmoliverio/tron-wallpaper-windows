#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

// graphics library mathematics :) 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// #include <shader_utils.hpp>
#include "shader.hpp"

// #define WIDTH 960
int WIDTH = 1280;
int HEIGHT = 720;

float sqrt_of_2 = std::sqrt(2.0f);
float sqrt_of_3 = std::sqrt(3.0f);
float sqrt_of_6 = std::sqrt(6.0f);

float vertices[] = {
    // position x y z                            // colors
    0.0f, 1.0f, 0.0f,                               0.0f, 0.0f, 1.0f,
    sqrt_of_3 / 2.0f, -0.5f, 0.0f,      1.0f, 0.0f, 0.0f,
    -sqrt_of_3 / 2.0f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f
};

// float pyramid[] = {
//     // positions                     // colors 
//     0.0f, 1.0f, 0.0f,                   0.0f, 0.0f, 1.0f,
//     sqrt_of_3 / 2.0f, -0.5f, 0.0f,      1.0f, 0.0f, 0.0f,
//     -sqrt_of_3 / 2.0f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f
// };

float pyrdamid_vertices[] = {
    // x y z 
    0.0f, 0.0f, 1.0f,
    2.0f * sqrt_of_2 / 3.0f, 0.0f, -1.0f / 3.0f,
    -sqrt_of_2 / 3.0f, sqrt_of_6 / 3.0f, -1.0f / 3.0f,
    -sqrt_of_2 / 3.0f, -sqrt_of_6 / 3.0f, -1.0f / 3.0f,
};

unsigned int pyramid_indices[] = {
    // Side faces
    0, 1, 2,
    0, 2, 3,
    0, 3, 1,

    // Base face
    1, 3, 2
};


static void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
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
        WIDTH, HEIGHT,
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

    uint32_t VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(pyrdamid_vertices), 
        pyrdamid_vertices, 
        GL_STATIC_DRAW);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
        sizeof(pyramid_indices), 
        pyramid_indices, 
        GL_STATIC_DRAW);

    // Vertex attribute
    glVertexAttribPointer(
        0,                      // location
        3,                      // vec3
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // uint32_t VBO;
    // uint32_t VAO;
    // glGenBuffers(1, &VBO);
    // glGenVertexArrays(1, &VAO);

    // glBindVertexArray(VAO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // for(int i = 0; i < 4; i++){
    //     int first = (i + 0) * (i + 1);
    //     int second = (i + 1) * (i + 1);
    //     int third = (i + 2) * (i + 1);


    // }

    // move trinagle data in there
    // glBindVertexArray(VAO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glVertexAttribPointer(0,
    //                       3,
    //                       GL_FLOAT,
    //                       GL_FALSE,
    //                       3 * sizeof(float),
    //                       (void *)0);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(1,
    //                       3,
    //                       GL_FLOAT,
    //                       GL_FALSE,
    //                       3 * sizeof(float),
    //                       (void *)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    Shader normal_shader("vertex_shader.vert", "fragment_shader.frag");
    normal_shader.use();
    
    int32_t viewport_sizes = glGetUniformLocation(
        normal_shader.ID, 
        "viewport_size"
    );
    if(viewport_sizes == -1)
        std::cerr << "Did not find viewport size variable" << std::endl;

    glUniform2f(viewport_sizes, (float) WIDTH, (float) HEIGHT);

    float rot_angle = 0.0f;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.08f, 0.10f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        normal_shader.use();

        viewport_sizes = glGetUniformLocation(
            normal_shader.ID, 
            "viewport_size"
        );
        if(viewport_sizes == -1)
            std::cerr << "Did not find viewport size variable" << std::endl;
        glUniform2f(viewport_sizes, (float) WIDTH, (float) HEIGHT);

        glm::mat4 transform_matrix = glm::mat4(1.0f);
        transform_matrix = glm::scale(
            transform_matrix, 
            glm::vec3(1.0f, 1.0f, 1.0f)
        );
        transform_matrix = glm::rotate(
            transform_matrix,
            glm::radians(rot_angle),
            glm::vec3(rot_angle, 0.0f, rot_angle)
        );

        int32_t transform_uniform = glGetUniformLocation(
            normal_shader.ID,
            "transform"
        );

        if(transform_uniform == -1) {
            std::cerr << "Did not find a uniform" << std::endl;
        }
        transform_matrix = glm::scale(
            transform_matrix, glm::vec3(0.75f, 0.75f, 0.75f)
        );
        transform_matrix = glm::rotate(
            transform_matrix, 
            glm::radians(rot_angle),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glUniformMatrix4fv(
            transform_uniform, 
            1, 
            GL_FALSE, 
            glm::value_ptr(transform_matrix)
        );

        glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        rot_angle += 0.1f;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}