#include <windows.h>
#include <gl/GL.h>
#include <iostream>

#pragma comment(lib, "opengl32.lib")

static HWND g_workerw = nullptr;
static HDC g_hdc;
static HGLRC g_glrc;

void WaitForEnter()
{
    std::cout << "Press ENTER to continue...\n";
    std::cin.get();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateWorkerW()
{
    HWND progman = FindWindowW(L"Progman", nullptr);

    SendMessageTimeoutW(
        progman,
        0x052C,
        0,
        0,
        SMTO_NORMAL,
        100,
        nullptr
    );
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM)
{
    wchar_t className[256];
    GetClassNameW(hwnd, className, 256);

    // Case 1: Progman owns SHELLDLL_DefView
    if (wcscmp(className, L"Progman") == 0)
    {
        HWND defView = FindWindowExW(
            hwnd,
            nullptr,
            L"SHELLDLL_DefView",
            nullptr
        );

        if (defView)
        {
            // Wallpaper WorkerW is AFTER Progman
            g_workerw = FindWindowExW(
                nullptr,
                hwnd,
                L"WorkerW",
                nullptr
            );
            return FALSE;
        }
    }

    // Case 2: WorkerW owns SHELLDLL_DefView
    if (wcscmp(className, L"WorkerW") == 0)
    {
        HWND defView = FindWindowExW(
            hwnd,
            nullptr,
            L"SHELLDLL_DefView",
            nullptr
        );

        if (defView)
        {
            // Wallpaper WorkerW is AFTER this WorkerW
            g_workerw = FindWindowExW(
                nullptr,
                hwnd,
                L"WorkerW",
                nullptr
            );
            return FALSE;
        }
    }

    return TRUE;
}

HWND GetWallpaperWorkerW()
{
    WaitForEnter();
    EnumWindows(EnumWindowsProc, 0);
    std::cout << g_workerw << std::endl;
    WaitForEnter();
    return g_workerw;
}

HWND CreateGLWindow(HINSTANCE hInstance)
{
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DesktopGL";

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    return hwnd;
}

void InitOpenGL(HWND hwnd)
{
    g_hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    int pf = ChoosePixelFormat(g_hdc, &pfd);
    SetPixelFormat(g_hdc, pf, &pfd);

    g_glrc = wglCreateContext(g_hdc);
    wglMakeCurrent(g_hdc, g_glrc);
}

void Render()
{
    static float t = 0.0f;
    t += 0.01f;

    glViewport(0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN));

    glClearColor(
        0.2f + 0.2f * sinf(t),
        0.3f,
        0.6f,
        1.0f
    );

    glClear(GL_COLOR_BUFFER_BIT);
    SwapBuffers(g_hdc);
}

void AttachDebugConsole()
{
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$",  "r", stdin);

    SetConsoleTitleW(L"Desktop OpenGL Debug Console");

    std::cout.clear();
    std::cerr.clear();

    std::ios::sync_with_stdio(true);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    AttachDebugConsole();

    CreateWorkerW();

    HWND workerw = GetWallpaperWorkerW();
    if (!workerw)
        return -1;

    std::cout << "Got wallpaper worker" << std::endl;
    WaitForEnter();

    HWND hwnd = CreateGLWindow(hInstance);

    std::cout << "Mkae OpenGLWindow" << std::endl;
    WaitForEnter();

    // 🔥 THIS IS THE KEY LINE 🔥
    SetParent(hwnd, workerw);

    InitOpenGL(hwnd);

    MSG msg{};
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return 0;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Render();
        Sleep(16);
    }
}