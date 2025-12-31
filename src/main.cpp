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

// For DPI awareness functions
#include <shellscalingapi.h>
#pragma comment(lib, "Shlwapi.lib")

// For PathFindFileNameW
#include <shlwapi.h>
#pragma comment(lib, "Shcore.lib")

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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

#include <windows.h>
#include <iostream>

// Global handle to store the found ShellDLL_DefView
HWND hShellDefView = NULL;

// EnumChildWindows callback to locate the Desktop Icons window
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {
    LPSTR className;
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

HWND FindShellDefView(HWND progman)
{
    HWND shellView = nullptr;

    // 1) Try direct child
    shellView = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
    if (shellView)
        return shellView;

    // 2) Walk WorkerWs (some builds hide it there)
    HWND worker = nullptr;
    while ((worker = FindWindowExW(nullptr, worker, L"WorkerW", nullptr)))
    {
        shellView = FindWindowExW(worker, nullptr, L"SHELLDLL_DefView", nullptr);
        if (shellView)
            return shellView;
    }

    return nullptr;
}

HWND FindWorkerW(HWND progman)
{
    HWND worker = nullptr;

    while ((worker = FindWindowExW(nullptr, worker, L"WorkerW", nullptr)))
    {
        // We want the WorkerW *behind* icons
        if (!FindWindowExW(worker, nullptr, L"SHELLDLL_DefView", nullptr))
            return worker;
    }

    return nullptr;
}

bool HasExtendedStyle(HWND hwnd, DWORD exStyle)
{
    return (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & exStyle) != 0;
}

int main()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    HRESULT dpiAwarenessResult = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    if (FAILED(dpiAwarenessResult)) {
        // Continue if needed, but coordinate values may be scaled.
    }


    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman)
    {
        std::cerr << "Progman not found\n";
        return 0;
    }

    bool isRaisedDesktop = HasExtendedStyle(progman, WS_EX_NOREDIRECTIONBITMAP);
    std::cout << "Is raised desktop: " << (isRaisedDesktop ? "YES" : "NO") << std::endl;

    // Always send this — harmless on older desktops
    SendMessageTimeout(
        progman,
        0x052C, // Progman spawn WorkerW
        0, 0,
        SMTO_NORMAL,
        1000,
        nullptr
    );

    // Try to locate the Shell view (desktop icons) and WorkerW child directly under Progman
    HWND shellView = FindWindowEx(progman, NULL, "SHELLDLL_DefView", NULL);
    HWND workerW = FindWindowEx(progman, NULL, "WorkerW", NULL);

    std::cout << "ShellView: " << shellView << std::endl;
    std::cout << "WorkerW:   " << workerW   << std::endl;

    if (!shellView || !workerW)
    {
        std::cerr << "Failed to locate desktop components\n";
        return 0;
    }

    DWORD ex = WS_EX_LAYERED | WS_EX_NOACTIVATE;
    HWND hLiveWP = CreateWindowEx(
        ex,
        "LiveWPClass",
        "",
        WS_CHILD,
        0, 0, 2560, 1440,
        progman,
        nullptr,
        hInstance,
        nullptr
    );

    // Prepare the engine window to be a layered child of Progman
    LONG_PTR style = GetWindowLongPtr(hLiveWP, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW); // Remove decorations
    style |= WS_CHILD; // Child style required for SetParent
    SetWindowLongPtr(hLiveWP, GWL_STYLE, style);

    LONG_PTR exStyle = GetWindowLongPtr(hLiveWP, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED; // Make it a layered window for 24H2
    SetWindowLongPtr(hLiveWP, GWL_EXSTYLE, exStyle);
    SetLayeredWindowAttributes(hLiveWP, 0, 255, LWA_ALPHA);

    // Reparent the engine window directly to Progman
    SetParent(hLiveWP, progman);

    // Place wallpaper ABOVE icons
    SetWindowPos(
        hLiveWP,
        shellView,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );

    // Push WorkerW behind wallpaper
    SetWindowPos(
        workerW,
        hLiveWP,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );

    // Resize/reposition the engine window to match its new parent.
    // g_progmanWindowHandle spans the entire virtual desktop in modern builds
    // SetWindowPos(
    //     hLiveWP,
    //     NULL,
    //     0,
    //     0,
    //     2560,
    //     1440,
    //     SWP_NOZORDER | SWP_NOACTIVATE
    // );

    // RedrawWindow(hLiveWP, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    // ShowWindow(hLiveWP, SW_SHOW);

    std::cout << "GLFW window successfully attached under Progman\n";
    
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
        
        float time = (GetTickCount() - startTime) / 1000.0f;
        float r = (float)((sin(time * 0.5) + 1.0) * 0.5);
        float g = (float)((sin(time * 0.7) + 1.0) * 0.5);
        float b = (float)((sin(time * 0.3) + 1.0) * 0.5);
        
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Swap buffers to render
        // SwapBuffers(hdc);
        
        frameCount++;
        if (frameCount % 120 == 0)
        {
            std::cout << "Frame " << frameCount << " - Rendering...\n";
        }
        
        Sleep(16); // ~60 FPS
    }
    
}