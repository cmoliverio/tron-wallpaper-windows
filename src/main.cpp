#define GLFW_EXPOSE_NATIVE_WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>
#include <random>

// graphics library mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// For DPI awareness functions
#include <shellscalingapi.h>
#pragma comment(lib, "Shlwapi.lib")

// // For PathFindFileNameW
#include <shlwapi.h>
#pragma comment(lib, "Shcore.lib")

#include "shader.hpp"
#include "tetrahedron.hpp"
#include "capsule.hpp"
#include "light_cycle.hpp"

GLFWwindow *init_glfw_window(uint32_t width, uint32_t height)
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        std::exit(-1);
    }

    GLFWwindow *window = glfwCreateWindow(
        width,
        height,
        "GLFW Wallpaper",
        nullptr,
        nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::exit(-1);
    }

    // Make the window's context current.
    glfwMakeContextCurrent(window);

    // time to add hints, this makes it expand
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    int version = gladLoadGL();
    if (version == 0)
    {
        std::cerr << "Failed to load GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(-1);
    }

    return window;
}

HWND get_progman()
{
    HWND progman = FindWindow(TEXT("Progman"), NULL);
    if (!progman)
    {
        std::cerr << "Did not find progman.  Giving up." << std::endl;
        std::exit(-1);
    }
    return progman;
}

void init_os()
{
    // Set the process DPI awareness to get physical pixel coordinates.
    HRESULT result = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    if (FAILED(result))
    {
        // do nothing, scaling might be broken though
    }

    HWND progman = get_progman();

    // creates WorkerW window, required for pre 24H2, no effect post 24H2
    SendMessageTimeout(
        progman,
        0x052C, // UNDOCUMENTED, this is a magic number which can break anytime
        0,
        0,
        SMTO_NORMAL,
        1000,
        nullptr);
}

void attach_24h2_or_newer(HWND window, uint32_t width, uint32_t height)
{
    HWND progman = get_progman();

    // prepare the engine window to be a layered child of Progman
    LONG_PTR style = GetWindowLongPtr(window, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW); // remove decorations
    style |= WS_CHILD;               // child required for SetParent
    SetWindowLongPtr(window, GWL_STYLE, style);

    LONG_PTR exStyle = GetWindowLongPtr(window, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED; // make it a layered window for 24H2 or newer
    SetWindowLongPtr(window, GWL_EXSTYLE, exStyle);

    // set fully opaque
    SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA);

    // parent to progman
    SetParent(window, progman);

    HWND shell = FindWindowEx(progman, NULL, TEXT("SHELLDLL_DefView"), NULL);
    if (!shell)
    {
        std::cerr << "WARNING: Missing SHELLDLL_DefView\n"
                  << "The wallpaper might cover desktop icons..."
                  << std::endl;
    }
    else
    {
        // move below SHELLDLL_DefView
        SetWindowPos(
            window,
            shell,
            0, 0,
            0, 0,
            (SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE));
    }

    // resize/reposition the engine window to match progman
    SetWindowPos(
        window,
        NULL,
        0,
        0,
        width,
        height,
        (SWP_NOZORDER | SWP_NOACTIVATE));
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    HWND shell = FindWindowEx(hwnd, NULL, TEXT("SHELLDLL_DefView"), NULL);
    if (shell != NULL)
    {
        HWND *workerW = reinterpret_cast<HWND *>(lParam);
        *workerW = FindWindowEx(NULL, hwnd, TEXT("WorkerW"), NULL);
        return FALSE; // stop
    }
    return TRUE; // continue
};

void attach_pre_24h2(HWND window)
{
    HWND workerW = nullptr;

    // find WorkerW
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&workerW));
    if (!workerW)
    {
        std::cerr << "Failed to locate WorkerW\n";
        std::exit(-1);
    }

    // set extended style as child and remove overlapping
    LONG_PTR style = GetWindowLongPtr(window, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW);
    style |= WS_CHILD; // MUST be a child
    SetWindowLongPtr(window, GWL_STYLE, style);

    SetParent(window, workerW);
}

void attach_wallpaper_to_os(HWND window, uint32_t width, uint32_t height)
{
    HWND progman = get_progman();

    // this determines whether the windows version is before or after 24H2
    // since that is when Microsoft introduced breaking changes to the
    // window structure. this change included a critical style flag recognized
    // as WS_EX_NOREDIRECTIONBITMAP. run spyxx.exe for more info:
    // (comes with Visual Studio)
    // C:/Program Files/Microsoft Visual Studio/<vrsn>/<dstrbtn>/Common7/Tools/spyxx.exe
    LONG_PTR extended_style = GetWindowLongPtr(progman, GWL_EXSTYLE);
    bool is_raised_desktop = (extended_style & WS_EX_NOREDIRECTIONBITMAP) != 0;

    if (is_raised_desktop)
    {
        std::cout << "Assuming 24H2 Windows or newer..." << std::endl;
        attach_24h2_or_newer(window, width, height);
    }
    else
    {
        std::cout << "Assuming pre 24H2 Windows..." << std::endl;
        attach_pre_24h2(window);
    }
}

glm::vec3 randomUnitAxis()
{
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    glm::vec3 axis;

    do
    {
        axis = glm::vec3(
            dist(rng),
            dist(rng),
            dist(rng));
    } while (glm::dot(axis, axis) < 1e-6f); // use dot instead of length2

    return glm::normalize(axis);
}

int main()
{
    // prepare Windows
    init_os();

    // get (main) monitor dimensions
    uint32_t width = 1800;
    uint32_t height = 720;
    // uint32_t width  = GetSystemMetrics(SM_CXSCREEN);
    // uint32_t height = GetSystemMetrics(SM_CYSCREEN);

    GLFWwindow *window = init_glfw_window(width, height);

    // get the windows handle
    // HWND hwnd = glfwGetWin32Window(window);
    // if (!hwnd) {
    //     std::cerr << "Could not get Windows handle from GLFW" << std::endl;
    //     std::exit(-1);
    // }

    // set as background
    // attach_wallpaper_to_os(hwnd, width, height);

    Shader normal_shader(
        "vertex_shader.vert",
        "geometry_shader.geom",
        "fragment_shader.frag");
    normal_shader.use();

    uint32_t num_of_objs = 20;

    std::vector<Tetrahedron> tetrahedrons;
    tetrahedrons.reserve(num_of_objs);

    std::mt19937 rng{std::random_device{}()};

    // Spread in world space
    std::uniform_real_distribution<float> distXY(-5.0f, 5.0f);
    std::uniform_real_distribution<float> distZ(-15.f, -2.0f); // in front of camera at z=+3

    for (uint32_t i = 0; i < num_of_objs; ++i)
    {
        glm::vec3 pos{
            distXY(rng),
            distXY(rng),
            distZ(rng)};

        tetrahedrons.emplace_back(pos);
        tetrahedrons[i].scale(glm::vec3(0.30f, 0.30f, 0.30f));
    }

    std::vector<glm::vec3> spinAxes;
    spinAxes.reserve(num_of_objs);

    for (uint32_t i = 0; i < num_of_objs; ++i)
    {
        spinAxes.push_back(randomUnitAxis());
    }

    std::vector<float> spinSpeeds; // radians per frame
    spinSpeeds.reserve(num_of_objs);

    std::normal_distribution<float> spinDegDist(0.1f, 0.1f); // degrees
    // std::mt19937 rng{ std::random_device{}() };

    for (uint32_t i = 0; i < num_of_objs; ++i)
    {
        float spinDeg;

        // Draw until within desired range (truncated normal)
        do
        {
            spinDeg = spinDegDist(rng);
        } while (spinDeg < 0.01f || spinDeg > 0.2f);

        spinSpeeds.push_back(glm::radians(spinDeg)); // convert to radians
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // glDisable(GL_CULL_FACE);
    // glDisable(GL_DEPTH_TEST); // temporarily

    std::chrono::steady_clock::time_point t1;
    std::chrono::steady_clock::time_point t2;

    // light cycle shader
    Shader light_cycle_shader(
        "light_cycle_vertex_shader.vert",
        // "light_cycle_geometry_shader.geom",
        "light_cycle_fragment_shader.frag");

    // create a light cycle here
    LightCycle first_cycle(glm::vec3(-3.0f, -0.5f, -5.0f));


    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
    t1 = t0;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // measure time
        t2 = std::chrono::steady_clock::now();

        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.08f, 0.10f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        normal_shader.use();

        view = glm::translate(view, glm::vec3(-0.003f, 0.0f, 0.0f));
        // view
        int32_t view_loc = glGetUniformLocation(
            normal_shader.ID,
            "view");
        if (view_loc == -1)
        {
            std::cerr << "View uniform not found" << std::endl;
        }
        glUniformMatrix4fv(
            view_loc,
            1,
            GL_FALSE,
            glm::value_ptr(view));

        // projection
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)width / (float)height, // not sure what this is tbh
            0.1f,                         // near clipping distance
            100.0f                        // far clipping distance
        );
        int32_t projection_loc = glGetUniformLocation(
            normal_shader.ID,
            "projection");
        if (projection_loc == -1)
        {
            std::cerr << "Projection uniform not found" << std::endl;
        }
        glUniformMatrix4fv(
            projection_loc,
            1,
            GL_FALSE,
            glm::value_ptr(projection));

        for (uint32_t i = 0; i < tetrahedrons.size(); ++i)
        {
            // then get the value for that triangle here and make the spin speed
            tetrahedrons[i].rotate(spinSpeeds[i] * elapsed.count(), spinAxes[i]);
            tetrahedrons[i].draw(normal_shader, i);
        }

        // light_cycle_shader.use();

        // // view
        // // view = glm::mat4(1.0f);
        // // view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        // view_loc = glGetUniformLocation(
        //     light_cycle_shader.ID,
        //     "view");
        // if (view_loc == -1)
        // {
        //     std::cerr << "View uniform not found" << std::endl;
        // }
        // glUniformMatrix4fv(
        //     view_loc,
        //     1,
        //     GL_FALSE,
        //     glm::value_ptr(view));

        // // projection
        // projection = glm::perspective(
        //     glm::radians(45.0f),
        //     (float)width / (float)height, // not sure what this is tbh
        //     0.1f,                         // near clipping distance
        //     100.0f                        // far clipping distance
        // );
        // projection_loc = glGetUniformLocation(
        //     light_cycle_shader.ID,
        //     "projection");
        // if (projection_loc == -1)
        // {
        //     std::cerr << "Projection uniform not found" << std::endl;
        // }
        // glUniformMatrix4fv(
        //     projection_loc,
        //     1,
        //     GL_FALSE,
        //     glm::value_ptr(projection));
        light_cycle_shader.use();

        // Set uniforms for lighting
        glm::vec3 lightPosView = glm::vec3(view * glm::vec4(5.0f, 5.0f, 5.0f, 1.0f));
        int32_t light_pos_loc = glGetUniformLocation(light_cycle_shader.ID, "uLightPos");
        if (light_pos_loc != -1) {
            glUniform3fv(light_pos_loc, 1, glm::value_ptr(lightPosView));
        }

        int32_t color_loc = glGetUniformLocation(light_cycle_shader.ID, "uColor");
        if (color_loc != -1) {
            glUniform3f(color_loc, 0.0f, 0.8f, 1.0f); // Cyan/blue color for the light cycle
        }

        // int32_t ambient_loc = glGetUniformLocation(light_cycle_shader.ID, "uAmbient");
        // if (ambient_loc != -1) {
        //     glUniform1f(ambient_loc, 0.3f);
        // }

        // view matrix
        view_loc = glGetUniformLocation(light_cycle_shader.ID, "view");
        if (view_loc != -1) {
            glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        }

        // projection matrix
        projection_loc = glGetUniformLocation(light_cycle_shader.ID, "projection");
        if (projection_loc != -1) {
            glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        auto total_time_elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t0);

        static int turnStage = 0;
        const auto t = total_time_elapsed.count();

        switch (turnStage)
        {
            case 0:
                if (t > 1000) {
                    first_cycle.change_direction(Direction::Down);
                    turnStage++;
                    first_cycle.print_points();
                }
                break;

            case 1:
                if (t > 2000) {
                    first_cycle.change_direction(Direction::Forward);
                    turnStage++;
                    first_cycle.print_points();
                }
                break;

            case 2:
                if (t > 3000) {
                    first_cycle.change_direction(Direction::Right);
                    turnStage++;
                    first_cycle.print_points();
                }
                break;

            case 3:
                if (t > 5000) {
                    first_cycle.change_direction(Direction::Backward);
                    turnStage++;
                    first_cycle.print_points();
                }
                break;

            case 4:
                if (t > 6000) {
                    first_cycle.change_direction(Direction::Right);
                    turnStage++;
                    first_cycle.print_points();
                }
                break;
            case 5:
                if (t > 7000) {
                    first_cycle.change_direction(Direction::Up);
                    turnStage++;
                    first_cycle.print_points();
                }
                break;
            case 6:
                if (t > 8500) {
                    first_cycle.change_direction(Direction::Right);
                    turnStage++;
                    first_cycle.print_points();
                }
                break;
        }

        // start rendering the light cycles here
        first_cycle.move(elapsed.count());
        first_cycle.draw(light_cycle_shader);

        glfwSwapBuffers(window);
        glfwPollEvents();

        t1 = t2;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}