#include "wallpaper_utils.hpp"
#include <iostream>

LRESULT CALLBACK WallpaperWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        return 0; // ignore close
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

void SpawnWorkerW()
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

HWND GetWorkerW()
{
    HWND workerw = nullptr;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
    {
        HWND defView = FindWindowExW(
            hwnd,
            nullptr,
            L"SHELLDLL_DefView",
            nullptr
        );

        if (defView)
        {
            HWND next = FindWindowExW(
                nullptr,
                hwnd,
                L"WorkerW",
                nullptr
            );

            if (next)
            {
                *reinterpret_cast<HWND*>(lParam) = next;
                return FALSE;
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&workerw));

    return workerw;
}

void AttachGLFWWindowToWallpaper(HWND hwnd)
{
    SpawnWorkerW();
    HWND workerw = GetWorkerW();

    if (!workerw)
    {
        MessageBoxW(nullptr, L"WorkerW not found", L"Error", MB_ICONERROR);
        return;
    }

    // Remove popup / overlapped styles
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUP);
    style |= WS_CHILD;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    // Extended styles
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
    exStyle &= ~WS_EX_APPWINDOW;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

    // Parent to WorkerW
    SetParent(hwnd, workerw);

    // Resize to full screen
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);

    SetWindowPos(
        hwnd,
        HWND_BOTTOM,
        0, 0,
        w, h,
        SWP_NOACTIVATE | SWP_SHOWWINDOW
    );
}