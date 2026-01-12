#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "Config.hpp"

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(450, 650, "MCP - Master Control Program", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Customize colors for a Tron-like aesthetic
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.95f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.3f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.5f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.14f, 0.8f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.3f, 0.9f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.4f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.6f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.14f, 0.8f, 1.0f, 1.0f);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load config
    Config config;
    config.load("tron_config.ini");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create main control window
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(450, 650), ImGuiCond_Always);
        ImGui::Begin("MASTER CONTROL PROGRAM", nullptr, 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("TRON WALLPAPER CONTROL INTERFACE");
        ImGui::Separator();
        
        bool changed = false;
        
        ImGui::Spacing();
        ImGui::Text("COLOR SETTINGS");
        ImGui::Separator();
        
        // Color sliders (0-255 for better UX)
        float color_r_255 = config.color_red * 255.0f;
        float color_g_255 = config.color_green * 255.0f;
        float color_b_255 = config.color_blue * 255.0f;
        
        if (ImGui::SliderFloat("Red", &color_r_255, 0.0f, 255.0f, "%.0f")) {
            config.color_red = color_r_255 / 255.0f;
            changed = true;
        }
        if (ImGui::SliderFloat("Green", &color_g_255, 0.0f, 255.0f, "%.0f")) {
            config.color_green = color_g_255 / 255.0f;
            changed = true;
        }
        if (ImGui::SliderFloat("Blue", &color_b_255, 0.0f, 255.0f, "%.0f")) {
            config.color_blue = color_b_255 / 255.0f;
            changed = true;
        }

        // Color preview
        ImVec4 preview_color(config.color_red, config.color_green, config.color_blue, 1.0f);
        ImGui::ColorButton("Color Preview", preview_color, 0, ImVec2(410, 30));

        ImGui::Spacing();
        ImGui::Text("SPEED SETTINGS");
        ImGui::Separator();
        
        if (ImGui::SliderFloat("Simulation Speed", &config.simulation_speed, 0.1f, 10.0f, "%.1fx")) {
            changed = true;
        }
        if (ImGui::SliderFloat("Cycle Speed", &config.cycle_speed, 0.1f, 10.0f, "%.1fx")) {
            changed = true;
        }
        if (ImGui::SliderFloat("Speed Randomness", &config.cycle_speed_randomness, 0.0f, 2.0f, "%.2f")) {
            changed = true;
        }

        ImGui::Spacing();
        ImGui::Text("VISUAL SETTINGS");
        ImGui::Separator();
        
        if (ImGui::SliderFloat("Thickness", &config.thickness, 0.1f, 3.0f, "%.2f")) {
            changed = true;
        }
        if (ImGui::SliderFloat("Bloom Power", &config.bloom_power, 0.1f, 10.0f, "%.1f")) {
            changed = true;
        }
        if (ImGui::SliderFloat("Bloom Distance", &config.bloom_distance, 0.1f, 10.0f, "%.1f")) {
            changed = true;
        }
        if (ImGui::SliderFloat("Bloom Flicker", &config.bloom_flicker_intensity, 0.0f, 1.0f, "%.2f")) {
            changed = true;
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Auto-save when any change is made
        if (changed) {
            config.save("tron_config.ini");
            ImGui::TextColored(ImVec4(0.14f, 0.8f, 1.0f, 1.0f), "STATUS: CONFIGURATION SAVED");
        } else {
            ImGui::Text("STATUS: READY");
        }

        // Manual save/load buttons
        ImGui::Spacing();
        if (ImGui::Button("RELOAD FROM FILE", ImVec2(200, 30))) {
            config.load("tron_config.ini");
        }
        ImGui::SameLine();
        if (ImGui::Button("RESET TO DEFAULTS", ImVec2(200, 30))) {
            config = Config(); // Reset to defaults
            config.save("tron_config.ini");
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}