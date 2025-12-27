#include <glad/glad.h>
#include <windows.h>
#include <iostream>

#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L

// OpenGL context variables
HDC hdc = NULL;
HGLRC hglrc = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            // OpenGL rendering happens in main loop, not here
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main() {
    // Console for debugging
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    std::cout << "=== OpenGL Wallpaper (Raw Win32) Starting ===\n";
    
    // Find the WorkerW window
    HWND progman = FindWindow(L"Progman", NULL);
    if (!progman) {
        std::cerr << "Progman not found\n";
        return -1;
    }

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    
    HWND workerw = NULL;
    // LONG_PTR exStyle = GetWindowLongPtr(progman, GWL_EXSTYLE);
    // bool isRaisedDesktopWithLayeredShellView = (exStyle & WS_EX_NOREDIRECTIONBITMAP) != 0;
    
    // if (isRaisedDesktopWithLayeredShellView) {
    //     std::cout << "Raised desktop with layered ShellView detected.\n";
        
        // Find the existing WorkerW
        HWND existingWorkerW = FindWindowEx(progman, NULL, L"WorkerW", NULL);
        if (existingWorkerW) {
            std::cout << "Found existing WorkerW: " << existingWorkerW << "\n";
        }
        
        // Instead of using the existing WorkerW, let's CREATE a new WorkerW as a child of Progman
        // This new WorkerW will be our rendering target
        std::cout << "Creating NEW WorkerW window as child of Progman...\n";
        
        WNDCLASSEX workerWC = {};
        workerWC.cbSize = sizeof(WNDCLASSEX);
        workerWC.style = CS_HREDRAW | CS_VREDRAW;
        workerWC.lpfnWndProc = DefWindowProc;  // Use default
        workerWC.hInstance = GetModuleHandle(NULL);
        workerWC.lpszClassName = L"WorkerW";  // Use the same class name as Windows' WorkerW
        
        // Try to register (might fail if already registered, that's ok)
        RegisterClassEx(&workerWC);
        
        // Create our own WorkerW as a child of Progman
        workerw = CreateWindowEx(
            0,                          // Extended style
            L"WorkerW",                  // Class name - same as Windows uses
            L"this is the freaking title",                         // No title
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,  // Style
            0, 0,                       // Position
            screen_width, screen_height,// Size
            progman,                    // Parent - Progman!
            NULL,                       // Menu
            GetModuleHandle(NULL),     // Instance
            NULL                        // Additional data
        );
        
        if (workerw) {
            std::cout << "Created NEW WorkerW: " << workerw << " as child of Progman: " << progman << "\n";
            
            // Find SHELLDLL_DefView to position our WorkerW above it
            HWND shelldll = FindWindowEx(progman, NULL, "SHELLDLL_DefView", NULL);
            if (shelldll) {
                std::cout << "Found SHELLDLL_DefView: " << shelldll << "\n";
                // Position our new WorkerW ABOVE (in front of) SHELLDLL_DefView
                SetWindowPos(workerw, shelldll, 0, 0, 0, 0, 
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                std::cout << "Positioned new WorkerW ABOVE SHELLDLL_DefView (in front of icons)\n";
            } else if (existingWorkerW) {
                // Fallback: position behind existing WorkerW
                SetWindowPos(workerw, existingWorkerW, 0, 0, 0, 0, 
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                std::cout << "Positioned new WorkerW behind existing WorkerW\n";
            }
            
            ShowWindow(workerw, SW_SHOW);
            UpdateWindow(workerw);
            
        } else {
            std::cerr << "Could not create new WorkerW. Error: " << GetLastError() << "\n";
            return -1;
        }
    // } else {
    //     std::cerr << "Non-raised desktop not implemented yet\n";
    //     return -1;
    // }
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "WallpaperGLClass";
    
    if (!RegisterClassEx(&wc)) {
        std::cerr << "Failed to register window class\n";
        return -1;
    }
    
    
    // Create window as child of WorkerW from the start
    HWND hwnd = CreateWindowEx(
        WS_EX_NOACTIVATE,           // Extended style
        "WallpaperGLClass",         // Class name
        "Wallpaper",                // Window name
        WS_CHILD | WS_VISIBLE,      // Style - child and visible from creation
        0, 0,                       // Position
        screen_width, screen_height,// Size
        workerw,                    // Parent - WorkerW!
        NULL,                       // Menu
        hInstance,                  // Instance
        NULL                        // Additional data
    );
    
    if (!hwnd) {
        std::cerr << "Failed to create window. Error: " << GetLastError() << "\n";
        return -1;
    }
    
    std::cout << "Created window: " << hwnd << "\n";
    std::cout << "Parent is: " << GetParent(hwnd) << " (should be " << workerw << ")\n";
    std::cout << "IsWindowVisible: " << IsWindowVisible(hwnd) << "\n";
    std::cout << "IsWindowEnabled: " << IsWindowEnabled(hwnd) << "\n";
    
    // Get window styles for debugging
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    LONG_PTR exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    std::cout << "Window style: 0x" << std::hex << style << std::dec << "\n";
    std::cout << "Extended style: 0x" << std::hex << exstyle << std::dec << "\n";
    
    // Get window rect
    RECT rect;
    GetWindowRect(hwnd, &rect);
    std::cout << "Window rect: " << rect.left << "," << rect.top << " to " 
              << rect.right << "," << rect.bottom << "\n";
    
    // Get device context
    hdc = GetDC(hwnd);
    if (!hdc) {
        std::cerr << "Failed to get DC\n";
        return -1;
    }
    std::cout << "Got DC: " << hdc << "\n";
    
    // Test: Try drawing with GDI first to see if THAT works
    std::cout << "Testing GDI drawing...\n";
    HBRUSH blueBrush = CreateSolidBrush(RGB(50, 150, 255));
    RECT fillRect = {0, 0, screen_width, screen_height};
    FillRect(hdc, &fillRect, blueBrush);
    DeleteObject(blueBrush);
    
    // Draw some text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    HFONT hFont = CreateFont(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    TextOut(hdc, 100, 100, "WALLPAPER TEST", 14);
    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
    
    std::cout << "GDI drawing complete. Do you see blue background with white text?\n";
    std::cout << "Waiting 5 seconds...\n";
    Sleep(5000);
    
    std::cout << "Now setting up OpenGL...\n";
    
    // Set pixel format
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (!pixelFormat) {
        std::cerr << "Failed to choose pixel format\n";
        return -1;
    }
    
    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        std::cerr << "Failed to set pixel format\n";
        return -1;
    }
    
    // Create OpenGL context
    hglrc = wglCreateContext(hdc);
    if (!hglrc) {
        std::cerr << "Failed to create OpenGL context\n";
        return -1;
    }
    
    if (!wglMakeCurrent(hdc, hglrc)) {
        std::cerr << "Failed to make OpenGL context current. Error: " << GetLastError() << "\n";
        return -1;
    }
    
    std::cout << "OpenGL context created successfully\n";
    std::cout << "wglGetCurrentContext: " << wglGetCurrentContext() << "\n";
    std::cout << "wglGetCurrentDC: " << wglGetCurrentDC() << "\n";
    
    // Initialize GLAD
    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    
    std::cout << "GLAD initialized. OpenGL " << glGetString(GL_VERSION) << "\n";
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Show window
    ShowWindow(hwnd, SW_SHOW);  // Changed to SW_SHOW instead of SW_SHOWNOACTIVATE
    UpdateWindow(hwnd);
    
    // Try bringing it to the top first to see if it appears at all
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    
    std::cout << "Set window to HWND_TOP temporarily to test visibility\n";
    std::cout << "Do you see ANYTHING on screen? (Press Ctrl+C after checking)\n";
    
    Sleep(3000); // Wait 3 seconds so you can check
    
    // Now put it at bottom
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    
    std::cout << "Moved window to HWND_BOTTOM\n";
    
    // Invalidate to force redraw
    InvalidateRect(hwnd, NULL, TRUE);
    
    // Force entire desktop to redraw
    InvalidateRect(NULL, NULL, TRUE);  // NULL = entire screen
    InvalidateRect(progman, NULL, TRUE);
    InvalidateRect(workerw, NULL, TRUE);
    
    // Try to force a desktop refresh
    HWND desktop = GetDesktopWindow();
    InvalidateRect(desktop, NULL, TRUE);
    UpdateWindow(desktop);
    
    std::cout << "Window shown and desktop invalidated.\n";
    std::cout << "Starting render loop...\n";
    
    // Main render loop
    MSG msg = {};
    int frameCount = 0;
    bool running = true;
    
    while (running) {
        // Process messages without blocking
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (!running) break;
        
        // Verify window is still valid
        if (!IsWindow(hwnd)) {
            std::cerr << "Window became invalid!\n";
            break;
        }
        
        // Render
        glViewport(0, 0, screen_width, screen_height);
        glClearColor(0.2f, 0.6f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw a simple triangle to test if OpenGL is working
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        glVertex2f(0.0f, 0.5f);
        glColor3f(0.0f, 1.0f, 0.0f); // Green
        glVertex2f(-0.5f, -0.5f);
        glColor3f(0.0f, 0.0f, 1.0f); // Blue
        glVertex2f(0.5f, -0.5f);
        glEnd();
        
        // Swap buffers
        if (!SwapBuffers(hdc)) {
            std::cerr << "SwapBuffers failed! Error: " << GetLastError() << "\n";
        }
        
        // Debug output
        if (frameCount % 300 == 0) {
            std::cout << "Frame " << frameCount << " - Parent: " << GetParent(hwnd) 
                      << " Valid: " << IsWindow(hwnd) << "\n";
        }
        frameCount++;
        
        Sleep(16); // ~60 FPS
    }
    
    // Cleanup
    std::cout << "Cleaning up...\n";
    
    if (hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hglrc);
    }
    
    if (hdc) {
        ReleaseDC(hwnd, hdc);
    }
    
    if (hwnd) {
        DestroyWindow(hwnd);
    }
    
    std::cout << "Cleanup complete. Press any key to exit...\n";
    system("pause");
    
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    return main();
}