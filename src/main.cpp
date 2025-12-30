// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>

// #include <GL/gl.h>
// // #define GLFW_EXPOSE_NATIVE_WIN32
// // #include <glad/glad.h>
// // #include <GLFW/glfw3.h>
// // #include <GLFW/glfw3native.h>
// #include <string>
// #include <iterator>
// #include <fstream>
// #include <sstream>
// #include <iostream>
// #include <cmath>
// #include <vector>
// #include <random>

// // // graphics library mathematics :) 
// // #include <glm/glm.hpp>
// // #include <glm/gtc/matrix_transform.hpp>
// // #include <glm/gtc/type_ptr.hpp>
// #include <GL/glu.h>

// #include "wallpaper_utils.hpp"

// #pragma comment(lib, "opengl32.lib")
// #pragma comment(lib, "gdi32.lib")
// #include "shader.hpp"
// #include "tetrahedron.hpp"
//
// #include <windows.h>
// #include <GL/gl.h>
// #include <iostream>
// #include <cmath>  // For sin()
//
#include <windows.h>
#include <GL/gl.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <cmath>
#include "wallpaper_utils.hpp"

// #pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")

// #define WIDTH 960
// int WIDTH = 1280;
// int HEIGHT = 720;

// static void framebuffer_size_callback(GLFWwindow *, int width, int height)
// {
//     WIDTH = width;
//     HEIGHT = height;
//     glViewport(0, 0, width, height);
// }

// glm::vec3 randomUnitAxis()
// {
//     static std::mt19937 rng{ std::random_device{}() };
//     static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

//     glm::vec3 axis;

//     do {
//         axis = glm::vec3(
//             dist(rng),
//             dist(rng),
//             dist(rng));
//     } while (glm::dot(axis, axis) < 1e-6f); // use dot instead of length2

//     return glm::normalize(axis);
// }

// LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// {
//     switch (msg)
//     {
//         case WM_DESTROY:
//             PostQuitMessage(0);
//             return 0;
//         case WM_PAINT:
//         {
//             PAINTSTRUCT ps;
//             BeginPaint(hwnd, &ps);
//             EndPaint(hwnd, &ps);
//             return 0;
//         }
//         default:
//             return DefWindowProcW(hwnd, msg, wParam, lParam);
//     }
// }

#include <windows.h>
#include <iostream>

// Global handle to store the found ShellDLL_DefView
HWND hShellDefView = NULL;

// EnumChildWindows callback to locate the Desktop Icons window
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {
    char className;
    GetClassNameA(hwnd, className, 100);

    // Look for the class that holds desktop icons
    if (strcmp(className, "SHELLDLL_DefView") == 0) {
        hShellDefView = hwnd;
        return FALSE; // Stop enumerating
    }
    return TRUE;
}

void InjectWallpaperWindow(HWND hMyWallpaperWindow) {
    // 1. Find the Program Manager window
    HWND hProgman = FindWindowA("Progman", NULL);
    if (!hProgman) {
        std::cerr << "Error: Could not find Progman." << std::endl;
        return;
    }

    // 2. Trigger the "Raised Desktop" state (Optional but recommended)
    // This undocumented message ensures the desktop composition hierarchy is initialized.
    // Even in 24H2, this helps ensure Progman is ready to accept children.
    SendMessageTimeoutA(hProgman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    // 3. Locate the "SHELLDLL_DefView" (Desktop Icons)
    // In 24H2, this is often a direct child of Progman or the Desktop.
    // We search specifically under Progman first.
    EnumChildWindows(hProgman, EnumChildProc, 0);
    
    // Fallback: If not found under Progman, search the entire desktop
    if (!hShellDefView) {
        HWND hDesktop = GetDesktopWindow();
        EnumChildWindows(hDesktop, EnumChildProc, 0);
    }

    if (!hShellDefView) {
        std::cerr << "Error: Could not find SHELLDLL_DefView." << std::endl;
        return;
    }

    // 4. Configure Your Window Style
    // 24H2 requires the child to be WS_EX_LAYERED to compose correctly in the stack.
    LONG_PTR exStyle = GetWindowLongPtr(hMyWallpaperWindow, GWL_EXSTYLE);
    SetWindowLongPtr(hMyWallpaperWindow, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    
    // Ensure the window is visible/opaque
    // LWA_ALPHA with 255 makes it fully opaque.
    SetLayeredWindowAttributes(hMyWallpaperWindow, 0, 255, LWA_ALPHA);

    // 5. Re-parent Your Window to Progman
    // Instead of parenting to a WorkerW, we attach directly to the manager.
    SetParent(hMyWallpaperWindow, hProgman);

    // 6. Fix Z-Order Injection
    // Insert our window immediately BEHIND the icons (hShellDefView).
    // flags: Do not resize, do not move, do not activate (steal focus).
    SetWindowPos(hMyWallpaperWindow, 
                 hShellDefView, // Place *under* this handle
                 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

    std::cout << "Wallpaper injected successfully under SHELLDLL_DefView." << std::endl;
}

int main()
{
    // Set DPI awareness FIRST (before any window creation)
    typedef BOOL (WINAPI *SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT);
    HMODULE user32 = LoadLibraryA("user32.dll");
    if (user32)
    {
        auto pSetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContextProc)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
        if (pSetProcessDpiAwarenessContext)
        {
            // Set to per-monitor V2 (Windows 10 1703+)
            if (pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            {
                std::cout << "DPI Awareness set to Per-Monitor V2\n";
            }
            else
            {
                std::cout << "Failed to set DPI awareness, trying fallback...\n";
                // Fallback to per-monitor aware
                pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
            }
        }
    }
    // Setup console for debugging
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    std::cout << "=== OpenGL Wallpaper Starting ===\n";

    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // Register window class
    WNDCLASSW wc = {};
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyOpenGLClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    if (!RegisterClassW(&wc))
    {
        std::cerr << "Failed to register window class: " << GetLastError() << std::endl;
        return -1;
    }

    // Create window
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    
    HWND hwnd = CreateWindowExW(
        0,
        L"MyOpenGLClass",
        L"OpenGL Wallpaper",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        width, height,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hwnd)
    {
        std::cerr << "Failed to create window: " << GetLastError() << std::endl;
        return -1;
    }

    std::cout << "Window created: " << hwnd << std::endl;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Setup OpenGL context
    HDC hdc = GetDC(hwnd);  // <-- HDC defined here
    if (!hdc)
    {
        std::cerr << "Failed to get DC: " << GetLastError() << std::endl;
        return -1;
    }
    std::cout << "Got DC: " << hdc << std::endl;

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    if (!pf)
    {
        std::cerr << "ChoosePixelFormat failed: " << GetLastError() << std::endl;
        return -1;
    }
    std::cout << "Chose pixel format: " << pf << std::endl;

    if (!SetPixelFormat(hdc, pf, &pfd))
    {
        std::cerr << "SetPixelFormat failed: " << GetLastError() << std::endl;
        return -1;
    }
    std::cout << "Set pixel format\n";

    HGLRC glContext = wglCreateContext(hdc);
    if (!glContext)
    {
        std::cerr << "wglCreateContext failed: " << GetLastError() << std::endl;
        return -1;
    }
    std::cout << "Created GL context: " << glContext << std::endl;

    if (!wglMakeCurrent(hdc, glContext))
    {
        std::cerr << "wglMakeCurrent failed: " << GetLastError() << std::endl;
        return -1;
    }
    std::cout << "Made context current\n";

    // Test render
    std::cout << "Testing basic rendering...\n";
    ::glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    ::glClear(GL_COLOR_BUFFER_BIT);
    ::SwapBuffers(hdc);
    
    std::cout << "Test render complete - you should see RED screen for 2 seconds\n";

    // Attach to wallpaper
    std::cout << "\n=== Attaching to desktop ===\n";
    AttachGLFWWindowToWallpaper(hwnd);
    
    // Check if we're in layered mode
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    bool isLayered = (exStyle & WS_EX_LAYERED) != 0;
    
    std::cout << "Window is layered: " << (isLayered ? "YES" : "NO") << std::endl;
    
    // Get window dimensions
    RECT rcWindow;
    GetClientRect(hwnd, &rcWindow);
    width = rcWindow.right;
    height = rcWindow.bottom;
    
    std::cout << "\n=== Starting render loop ===\n";
    if (isLayered)
    {
        std::cout << "Using UpdateLayeredWindow mode\n";
    }
    
    MSG msg = {};
    bool running = true;
    int frameCount = 0;
    DWORD startTime = GetTickCount();
    
    while (running)
    {
        // Process messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running) break;

        // Render to OpenGL
        glViewport(0, 0, width, height);
        
        float time = (GetTickCount() - startTime) / 1000.0f;
        float r = (float)((sin(time * 0.5) + 1.0) * 0.5);
        float g = (float)((sin(time * 0.7) + 1.0) * 0.5);
        float b = (float)((sin(time * 0.3) + 1.0) * 0.5);
        
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Swap buffers to render
        SwapBuffers(hdc);
        
        // If we're in layered mode, we need to manually update the layered window
        if (isLayered)
        {
            // Update the layered window from our DC
            if (!UpdateLayeredWindowFromDC(hwnd, hdc, width, height))
            {
                if (frameCount % 120 == 0) // Only log occasionally
                {
                    std::cerr << "UpdateLayeredWindow failed\n";
                }
            }
        }
        
        frameCount++;
        if (frameCount % 120 == 0)
        {
            std::cout << "Frame " << frameCount << " - Rendering...\n";
        }
        
        Sleep(16); // ~60 FPS
    }
    
    // ... cleanup ...
}