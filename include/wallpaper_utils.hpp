#ifndef WALLPAPER_UTILS_HPP
#define WALLPAPER_UTILS_HPP

#include <windows.h>

// Finds the WorkerW window behind desktop icons
HWND GetWorkerW();

// Forces Explorer to spawn the WorkerW
void SpawnWorkerW();

// Optional: custom WndProc if you want later
LRESULT CALLBACK WallpaperWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Attach a GLFW window to WorkerW and apply styles
void AttachGLFWWindowToWallpaper(HWND hwnd);

#endif // WALLPAPER_UTILS_HPP