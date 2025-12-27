#include <windows.h>
#include <gl/gl.h>
#include <iostream>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

// Find the WorkerW window behind the desktop icons
HWND GetWorkerW() {
    HWND progman = FindWindow("Progman", nullptr);
    HWND workerw = nullptr;

    // Send message to Progman to spawn a WorkerW behind icons
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    // Enumerate windows to find the WorkerW
    EnumWindows([](HWND topWnd, LPARAM lParam) -> BOOL {
        HWND shellView = FindWindowEx(topWnd, nullptr, "SHELLDLL_DefView", nullptr);
        if (shellView) {
            HWND* pWorker = (HWND*)lParam;
            *pWorker = FindWindowEx(nullptr, topWnd, "WorkerW", nullptr);
        }
        return TRUE;
    }, (LPARAM)&workerw);

    return workerw;
}

int main() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    std::cout << "=== OpenGL Wallpaper Starting ===\n";

    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // -------------------------------
    // Register window class
    // -------------------------------
    // const wchar_t* className = "MyOpenGLClass";
    WNDCLASSW wc = {};
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyOpenGLClass";

    RegisterClassW(&wc);

    // -------------------------------
    // Create window (borderless, no activate)
    // -------------------------------
    // HWND workerw = GetWorkerW();
    // if (!workerw) {
    //     std::cerr << "Failed to find WorkerW\n";
    //     return -1;
    // }

    HWND hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,  // hide from Alt+Tab + no activation
        L"MyOpenGLClass",
        L"My OpenGL Wallpaper",
        WS_POPUP | WS_VISIBLE,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL,  // parent to WorkerW
        nullptr,
        hInstance,
        nullptr
    );

    // Push the window to the bottom so it never covers other windows
    SetWindowPos(
        hwnd,
        HWND_BOTTOM,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW
    );

    HDC hdc = GetDC(hwnd);

    // -------------------------------
    // Set pixel format
    // -------------------------------
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);

    HGLRC glContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, glContext);

    // -------------------------------
    // Render loop
    // -------------------------------
    MSG msg = {};
    while (true) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto cleanup;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        int w = GetSystemMetrics(SM_CXSCREEN);
        int h = GetSystemMetrics(SM_CYSCREEN);

        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.6f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SwapBuffers(hdc);

        Sleep(1);
    }

cleanup:
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(glContext);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    return 0;
}