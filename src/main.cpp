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

GLFWwindow *init_glfw_window(uint32_t width, uint32_t height)
{
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
        std::exit(-1);
	}

	GLFWwindow *window = glfwCreateWindow(
		width, 
        height, 
        "GLFW Wallpaper", 
        nullptr, 
        nullptr
	);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::exit(-1);
	}

	// Make the window's context current.
	glfwMakeContextCurrent(window);

    // time to add hints, this makes it expand
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    int version = gladLoadGL();
    if (version == 0) {
        std::cerr << "Failed to load GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(-1);
    }

    return window;
}

HWND get_progman()
{
    HWND progman = FindWindow("Progman", NULL);
    if (!progman) { 
        std::cerr << "Did not find progman.  Giving up." << std::endl;
        std::exit(-1);
    }
    return progman;
}

void init_os()
{
    // Set the process DPI awareness to get physical pixel coordinates.
    HRESULT result = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    if (FAILED(result)) {
        // do nothing, scaling might be broken though
    }

    HWND progman = get_progman();

    // creates WorkerW window, required for pre 24H2, no effect post 24H2
    SendMessageTimeout(
        progman,
        0x052C, // UNDOCUMENTED, this is a magic number which can break anytime 
        0, 
        0,
        SMTO_NORMAL,
        1000,
        nullptr
    );
}

void attach_wallpaper_to_os(HWND window, uint32_t width, uint32_t height)
{
    HWND progman = get_progman();

    // this determines whether the windows version is before or after 24H2
    // since that is when Microsoft introduced breaking changes to the 
    // window structure. this change included a critical style flag recognized 
    // as WS_EX_NOREDIRECTIONBITMAP. run spyxx.exe for more info:
    // (comes with Visual Studio)
    // C:/Program Files/Microsoft Visual Studio/<vrsn>/<dstrbtn>/Common7/Tools/spyxx.exe
    bool is_raised_desktop = GetWindowLongPtr(progman, GWL_EXSTYLE);
    is_raised_desktop &= WS_EX_NOREDIRECTIONBITMAP;

    if (is_raised_desktop) {
        std::cout << "Assuming 24H2 Windows or newer..."  << std::endl;
        HWND shell = FindWindowEx(progman, NULL, "SHELLDLL_DefView", NULL);
        if (!shell) { 
            std::cerr << "WARNING: Missing SHELLDLL_DefView" << std::endl;
        }

        HWND workerW = FindWindowEx(progman, NULL, "WorkerW", NULL);
        if (!workerW) { 
            std::cerr << "WARNING: Missing WorkerW" << std::endl;
        }

        // Prepare the engine window to be a layered child of Progman
        LONG_PTR style = GetWindowLongPtr(window, GWL_STYLE);
        style &= ~(WS_OVERLAPPEDWINDOW); // remove decorations
        style |= WS_CHILD; // child required for SetParent
        SetWindowLongPtr(window, GWL_STYLE, style);

        LONG_PTR exStyle = GetWindowLongPtr(window, GWL_EXSTYLE);
        exStyle |= WS_EX_LAYERED; // Make it a layered window for 24H2
        SetWindowLongPtr(window, GWL_EXSTYLE, exStyle);

        // set fully opaque
        SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA);

        // parent to progman
        SetParent(window, progman);

        // move below SHELLDLL_DefView
        SetWindowPos(
            window,
            shell,
            0, 0, 
            0, 0,
            (SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)
        );

        // move worker below our window
        SetWindowPos(
            workerW,
            window,
            0, 0, 
            0, 0,
            (SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)
        );

        // resize/reposition the engine window to match progman
        SetWindowPos(
            window,
            NULL,
            0,
            0,
            width,
            height,
            (SWP_NOZORDER | SWP_NOACTIVATE)
        );
    } else {
        std::cout << "Assuming pre 24H2 Windows..."  << std::endl;
    }
}

int main()
{
    // prepare Windows
    init_os();
    
    // get (main) monitor dimensions
    uint32_t width  = GetSystemMetrics(SM_CXSCREEN);
    uint32_t height = GetSystemMetrics(SM_CYSCREEN);

    GLFWwindow *window = init_glfw_window(width, height);
    
    // get the windows handle
    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) {
        std::cerr << "Could not get Windows handle from GLFW" << std::endl;
        std::exit(-1);
    }

    // set as background
    attach_wallpaper_to_os(hwnd, width, height);

    // stop for now and cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    std::exit(0);

    // // Try to locate the Shell view (desktop icons) and WorkerW child directly under Progman
    // HWND shellView = FindWindowEx(progman, NULL, "SHELLDLL_DefView", NULL);
    // // HWND workerW = FindWindowEx(progman, NULL, "WorkerW", NULL);

    // std::cout << "ShellView: " << shellView << std::endl;
    // std::cout << "WorkerW:   " << workerW   << std::endl;

    // if (!shellView)
    // {
    //     std::cerr << "Failed to locate desktop components\n";
    //     return 0;
    // }


    // // Prepare the engine window to be a layered child of Progman
    // LONG_PTR style = GetWindowLongPtr(hLiveWP, GWL_STYLE);
    // style &= ~(WS_OVERLAPPEDWINDOW); // Remove decorations
    // style |= WS_CHILD; // Child style required for SetParent
    // SetWindowLongPtr(hLiveWP, GWL_STYLE, style);

    // LONG_PTR exStyle = GetWindowLongPtr(hLiveWP, GWL_EXSTYLE);
    // exStyle |= WS_EX_LAYERED; // Make it a layered window for 24H2
    // SetWindowLongPtr(hLiveWP, GWL_EXSTYLE, exStyle);
    // // SetLayeredWindowAttributes(hLiveWP, 0, 255, LWA_ALPHA);

    // // Reparent the engine window directly to Progman
    // SetParent(hLiveWP, progman);

    // // Place wallpaper ABOVE icons
    // SetWindowPos(
    //     hLiveWP,
    //     shellView,
    //     0, 0, 0, 0,
    //     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    // );

    // // // Push WorkerW behind wallpaper
    // // SetWindowPos(
    // //     workerW,
    // //     hLiveWP,
    // //     0, 0, 0, 0,
    // //     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    // // );

    // // Resize/reposition the engine window to match its new parent.
    // // g_progmanWindowHandle spans the entire virtual desktop in modern builds
    // SetWindowPos(
    //     hLiveWP,
    //     NULL,
    //     0,
    //     0,
    //     2560,
    //     1440,
    //     SWP_NOZORDER | SWP_NOACTIVATE
    // );

    // // RedrawWindow(hLiveWP, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    // // ShowWindow(hLiveWP, SW_SHOW);
    // // glfwSwapInterval(1);

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