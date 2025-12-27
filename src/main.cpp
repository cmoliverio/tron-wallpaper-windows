#include <glad/glad.h>  // MUST be included first, before any OpenGL headers
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <iostream>

HWND find_workerw() {
    HWND progman = FindWindow("Progman", NULL);
    if (!progman) {
        std::cerr << "Progman not found\n";
        return NULL;
    }

    // Send 0x052C message to spawn a WorkerW behind the desktop icons
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    // Now we need to find the WorkerW that was created
    // The correct one is the NEXT WorkerW after the one containing SHELLDLL_DefView
    HWND workerw_ret = NULL;
    
    EnumWindows([](HWND top_window, LPARAM lparam) -> BOOL {
        // Find the WorkerW that contains SHELLDLL_DefView (desktop icons)
        HWND shelldll_defview = FindWindowEx(top_window, NULL, "SHELLDLL_DefView", NULL);
        
        if (shelldll_defview != NULL) {
            // Found the WorkerW with icons, now get the next WorkerW
            // This next WorkerW is behind the icons
            HWND workerw_behind = FindWindowEx(NULL, top_window, "WorkerW", NULL);
            
            if (workerw_behind) {
                *((HWND*)lparam) = workerw_behind;
                std::cout << "Found WorkerW behind icons: " << workerw_behind << "\n";
                return FALSE; // Stop enumeration
            }
        }
        return TRUE; // Continue enumeration
    }, (LPARAM)&workerw_ret);

    if (!workerw_ret) {
        std::cerr << "Could not find the wallpaper WorkerW\n";
    }

    return workerw_ret;
}

int main() {
    // Console for debugging
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    std::cout << "=== OpenGL Wallpaper Starting ===\n";

    // -------------------------------
    // Initialize GLFW
    // -------------------------------
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);

    int screen_width  = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    // -------------------------------
    // Create GLFW window (not shown yet)
    // -------------------------------
    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Wallpaper", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // -------------------------------
    // Make context current + load GL
    // -------------------------------
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // -------------------------------
    // Get HWND from GLFW
    // -------------------------------
    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        std::cerr << "Failed to get HWND from GLFW window\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    std::cout << "Got HWND: " << hwnd << "\n";

    // -------------------------------
    // Find WorkerW behind desktop icons
    // -------------------------------
    HWND workerw = find_workerw();
    if (!workerw) {
        std::cerr << "Could not find WorkerW. Running as normal window.\n";
        ShowWindow(hwnd, SW_SHOW);
    } else {
        std::cout << "Found WorkerW: " << workerw << "\n";

        // -------------------------------
        // Fix window styles for wallpaper mode
        // -------------------------------
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style &= ~(WS_POPUP | WS_OVERLAPPEDWINDOW);
        style |= WS_CHILD;
        SetWindowLong(hwnd, GWL_STYLE, style);

        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        exStyle &= ~(WS_EX_APPWINDOW);
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

        // -------------------------------
        // Reparent BEFORE showing
        // -------------------------------
        if (!SetParent(hwnd, workerw)) {
            std::cerr << "SetParent failed. Error: " << GetLastError() << "\n";
        }

        // -------------------------------
        // Show window inside WorkerW
        // -------------------------------
        SetWindowPos(hwnd, NULL, 0, 0, screen_width, screen_height,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        std::cout << "Wallpaper window attached successfully.\n";
    }

    // -------------------------------
    // Render loop
    // -------------------------------
    std::cout << "Entering render loop...\n";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w == 0 || h == 0)
            continue;

        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.6f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        Sleep(1);
    }

    // -------------------------------
    // Cleanup
    // -------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Cleanup complete.\n";
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    return main();
}