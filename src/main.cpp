#define GLFW_EXPOSE_NATIVE_WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <string>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <random>

// graphics library mathematics :) 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "wallpaper_utils.hpp"
#include "shader.hpp"
#include "tetrahedron.hpp"

// #define WIDTH 960
int WIDTH = 1280;
int HEIGHT = 720;

static void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

glm::vec3 randomUnitAxis()
{
    static std::mt19937 rng{ std::random_device{}() };
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    glm::vec3 axis;

    do {
        axis = glm::vec3(
            dist(rng),
            dist(rng),
            dist(rng));
    } while (glm::dot(axis, axis) < 1e-6f); // use dot instead of length2

    return glm::normalize(axis);
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
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        std::cerr << "Failed to get primary monitor\n";
        glfwTerminate();
        return -1;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode)
    {
        std::cerr << "Failed to get video mode\n";
        glfwTerminate();
        return -1;
    }

    WIDTH  = mode->width;
    HEIGHT = mode->height;

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

    HWND hwnd = glfwGetWin32Window(window);
    AttachGLFWWindowToWallpaper(hwnd);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";

    Shader normal_shader(
        "vertex_shader.vert", 
        "geometry_shader.geom",
        "fragment_shader.frag"
    );
    normal_shader.use();

    uint32_t num_of_objs = 200;

    std::vector<Tetrahedron> tetrahedrons;
    tetrahedrons.reserve(num_of_objs);

    std::mt19937 rng{ std::random_device{}() };

    // Spread in world space
    std::uniform_real_distribution<float> distXY(-5.0f, 5.0f);
    std::uniform_real_distribution<float> distZ(-15.f, -2.0f); // in front of camera at z=+3

    for (int i = 0; i < num_of_objs; ++i)
    {
        glm::vec3 pos{
            distXY(rng),
            distXY(rng),
            distZ(rng)
        };

        tetrahedrons.emplace_back(pos);
        tetrahedrons[i].scale(glm::vec3(0.30f, 0.30f, 0.30f));
    }

    std::vector<glm::vec3> spinAxes;
    spinAxes.reserve(num_of_objs);

    for (int i = 0; i < num_of_objs; ++i)
    {
        spinAxes.push_back(randomUnitAxis());
    }

    std::vector<float> spinSpeeds;  // radians per frame
    spinSpeeds.reserve(num_of_objs);

    std::normal_distribution<float> spinDegDist(0.1f, 0.1f); // degrees
    // std::mt19937 rng{ std::random_device{}() };

    for (int i = 0; i < num_of_objs; ++i)
    {
        float spinDeg;

        // Draw until within desired range (truncated normal)
        do {
            spinDeg = spinDegDist(rng);
        } while (spinDeg < 0.01f || spinDeg > 0.2f);

        spinSpeeds.push_back(glm::radians(spinDeg)); // convert to radians
    }

    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.08f, 0.10f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        normal_shader.use();

        // view 
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        int32_t view_loc = glGetUniformLocation(
            normal_shader.ID,
            "view"
        );
        if (view_loc == -1) {
            std::cerr << "View uniform not found" << std::endl;
        }
        glUniformMatrix4fv(
            view_loc,
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        // projection
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 
            (float)WIDTH / (float)HEIGHT, // not sure what this is tbh
            0.1f,  // near clipping distance
            100.0f // far clipping distance
        );
        int32_t projection_loc = glGetUniformLocation(
            normal_shader.ID,
            "projection"
        );
        if (projection_loc == -1) {
            std::cerr << "Projection uniform not found" << std::endl;
        }
        glUniformMatrix4fv(
            projection_loc,
            1, 
            GL_FALSE,
            glm::value_ptr(projection)
        );

        constexpr float spin = glm::radians(0.5f);

        for (size_t i = 0; i < tetrahedrons.size(); ++i)
        {
            // then get the value for that triangle here and make the spin speed
            tetrahedrons[i].rotate(spinSpeeds[i], spinAxes[i]);
            tetrahedrons[i].draw(normal_shader, i);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}