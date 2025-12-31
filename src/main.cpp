#define GLFW_EXPOSE_NATIVE_WIN32
// #include <glad/glad.h>
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
	// Initialize GLFW
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

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

    // DWORD ex = WS_EX_LAYERED | WS_EX_NOACTIVATE;
    // HWND hLiveWP = CreateWindowEx(
    //     ex,
    //     "LiveWPClass",
    //     "",
    //     WS_CHILD,
    //     0, 0, 2560, 1440,
    //     progman,
    //     nullptr,
    //     hInstance,
    //     nullptr
    // );
    //
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

	HWND hLiveWP = glfwGetWin32Window(window);

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
    SetWindowPos(
        hLiveWP,
        NULL,
        0,
        0,
        2560,
        1440,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    RedrawWindow(hLiveWP, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    // ShowWindow(hLiveWP, SW_SHOW);
    glfwSwapInterval(1);

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

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    
}