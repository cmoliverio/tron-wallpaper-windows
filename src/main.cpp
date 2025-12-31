#define GLFW_EXPOSE_NATIVE_WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>

// For DPI awareness functions
#include <shellscalingapi.h>
#pragma comment(lib, "Shlwapi.lib")

// // For PathFindFileNameW
#include <shlwapi.h>
#pragma comment(lib, "Shcore.lib")

bool HasExtendedStyle(HWND hwnd, DWORD exStyle)
{
    return (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & exStyle) != 0;
}

int main()
{
	// Initialize GLFW
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// Create GLFW window with the size of the monitor.
	GLFWwindow *window = glfwCreateWindow(
		2560, 1440, "GLFW Wallpaper", nullptr, nullptr
	);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Make the window's context current.
	glfwMakeContextCurrent(window);

    int version = gladLoadGL();
    if (version == 0) {
        std::cerr << "Fucking broke" << std::endl;
        std::exit(-1);
    }

	HWND hLiveWP = glfwGetWin32Window(window);

	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // HINSTANCE hInstance = GetModuleHandle(nullptr);

    // HRESULT dpiAwarenessResult = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    // if (FAILED(dpiAwarenessResult)) {
    //     // Continue if needed, but coordinate values may be scaled.
    // }


    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman)
    {
        std::cerr << "Progman not found\n";
        return 0;
    }

    bool isRaisedDesktop = HasExtendedStyle(progman, WS_EX_NOREDIRECTIONBITMAP);
    std::cout << "Is raised desktop: " << (isRaisedDesktop ? "YES" : "NO") << std::endl;

    // Always send this — harmless on older desktops
    // SendMessageTimeout(
    //     progman,
    //     0x052C, // Progman spawn WorkerW
    //     0, 0,
    //     SMTO_NORMAL,
    //     1000,
    //     nullptr
    // );

    // Try to locate the Shell view (desktop icons) and WorkerW child directly under Progman
    HWND shellView = FindWindowEx(progman, NULL, "SHELLDLL_DefView", NULL);
    // HWND workerW = FindWindowEx(progman, NULL, "WorkerW", NULL);

    std::cout << "ShellView: " << shellView << std::endl;
    // std::cout << "WorkerW:   " << workerW   << std::endl;

    if (!shellView)
    {
        std::cerr << "Failed to locate desktop components\n";
        return 0;
    }


    // Prepare the engine window to be a layered child of Progman
    LONG_PTR style = GetWindowLongPtr(hLiveWP, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW); // Remove decorations
    style |= WS_CHILD; // Child style required for SetParent
    SetWindowLongPtr(hLiveWP, GWL_STYLE, style);

    LONG_PTR exStyle = GetWindowLongPtr(hLiveWP, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED; // Make it a layered window for 24H2
    SetWindowLongPtr(hLiveWP, GWL_EXSTYLE, exStyle);
    // SetLayeredWindowAttributes(hLiveWP, 0, 255, LWA_ALPHA);

    // Reparent the engine window directly to Progman
    SetParent(hLiveWP, progman);

    // Place wallpaper ABOVE icons
    SetWindowPos(
        hLiveWP,
        shellView,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );

    // // Push WorkerW behind wallpaper
    // SetWindowPos(
    //     workerW,
    //     hLiveWP,
    //     0, 0, 0, 0,
    //     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    // );

    // Resize/reposition the engine window to match its new parent.
    // g_progmanWindowHandle spans the entire virtual desktop in modern builds
    SetWindowPos(
        hLiveWP,
        NULL,
        0,
        0,
        2560,
        1440,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // RedrawWindow(hLiveWP, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    // ShowWindow(hLiveWP, SW_SHOW);
    // glfwSwapInterval(1);

    std::cout << "GLFW window successfully attached under Progman\n";
    
	while (!glfwWindowShouldClose(window)) {
		// // Animate background color to show frames are rendering.
		double timeSeconds = glfwGetTime();
		float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(timeSeconds) * 2.0f);
		glClearColor(0.08f + 0.12f * pulse, 0.15f + 0.25f * pulse, 0.25f + 0.35f * pulse, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Ensure viewport matches framebuffer size (HiDPI aware)
		int winW = 0, winH = 0;
		int fbW = 0, fbH = 0;
		glfwGetWindowSize(window, &winW, &winH);
		glfwGetFramebufferSize(window, &fbW, &fbH);
		glViewport(0, 0, fbW, fbH);


		glfwSwapBuffers(window);
		glfwPollEvents();

		std::this_thread::sleep_for(std::chrono::milliseconds(7));
	}
    
}