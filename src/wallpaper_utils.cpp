#include "wallpaper_utils.hpp"
#include <iostream>
#include <vector>
// #include <dwmapi.h>
// #pragma comment(lib, "dwmapi.lib")

bool HasExtendedStyle(HWND hwnd, DWORD style)
{
    if (hwnd == nullptr)
        return false;

    // Use GetWindowLongPtr for 64-bit and 32-bit compatibility
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exStyle == 0)
        return false;

    return (exStyle & style) != 0;
}

bool TrySetParent(HWND child, HWND parent)
{
    return SetParent(child, parent) != nullptr;
}

// Function to get the last child window of a parent
HWND GetLastChildWindow(HWND parent)
{
    HWND lastChild = nullptr;

    EnumChildWindows(parent, [](HWND hwnd, LPARAM lParam) -> BOOL {
        // lParam points to lastChild
        HWND* pLastChild = reinterpret_cast<HWND*>(lParam);
        *pLastChild = hwnd; // update lastChild
        return TRUE;        // continue enumeration
    }, reinterpret_cast<LPARAM>(&lastChild));

    return lastChild;
}

void EnsureWorkerWZOrder(HWND progman, HWND workerW, bool isRaisedDesktopWithLayeredShellView)
{
    if (!workerW || !isRaisedDesktopWithLayeredShellView)
        return;

    if (GetLastChildWindow(progman) != workerW)
    {
        std::cerr << "Unexpected WorkerW Z-order." << std::endl;

        UINT uFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;

        if (!SetWindowPos(workerW,
                          HWND_BOTTOM, // place at the bottom of Z-order
                          0, 0, 0, 0, // ignored because of SWP_NOMOVE | SWP_NOSIZE
                          uFlags))
        {
            std::cerr << "SetWindowPos failed. Error: " << GetLastError() << std::endl;
        }
    }
}

void SpawnWorkerW()
{
    HWND progman = FindWindowW(L"Progman", nullptr);
    
    DWORD_PTR result = 0;
    SendMessageTimeoutW(
        progman,
        0x052C,
        0xD,
        0x1,
        SMTO_NORMAL,
        1000,
        &result    // Capture the result
    );
    
    std::cout << "SendMessage result: " << result << std::endl;
}

HWND GetWallpaperWorkerW()
{
    HWND progman = FindWindowW(L"Progman", nullptr);
    HWND shellDLL_DefView = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
    
    // Check if raised desktop with layered ShellView
    bool isRaisedDesktop = HasExtendedStyle(progman, WS_EX_NOREDIRECTIONBITMAP);
    
    if (isRaisedDesktop)
    {
        // In raised desktop, WorkerW is a direct child of Progman
        HWND workerW = FindWindowExW(progman, nullptr, L"WorkerW", nullptr);
        std::cout << "Raised desktop detected, WorkerW (child of Progman): " << workerW << std::endl;
        return workerW;
    }
    else
    {
        // Original logic for older Windows versions
        HWND defViewParent = nullptr;
        
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            HWND defView = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
            if (defView)
            {
                *reinterpret_cast<HWND*>(lParam) = hwnd;
                return FALSE;
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&defViewParent));

        if (!defViewParent)
            return nullptr;

        // Find the next WorkerW after defViewParent
        HWND workerW = nullptr;
        bool foundDefViewParent = false;
        
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            auto* state = reinterpret_cast<std::pair<bool*, std::pair<HWND, HWND*>*>*>(lParam);
            bool& foundDefViewParent = *state->first;
            HWND defViewParent = state->second->first;
            HWND* pWorkerW = state->second->second;
            
            if (hwnd == defViewParent)
            {
                foundDefViewParent = true;
                return TRUE;
            }

            if (foundDefViewParent)
            {
                wchar_t cls[64];
                GetClassNameW(hwnd, cls, 64);
                if (wcscmp(cls, L"WorkerW") == 0)
                {
                    *pWorkerW = hwnd;
                    return FALSE;
                }
            }

            return TRUE;
        }, reinterpret_cast<LPARAM>(new std::pair<bool*, std::pair<HWND, HWND*>*>{
            &foundDefViewParent, 
            new std::pair<HWND, HWND*>{defViewParent, &workerW}
        }));

        std::cout << "Non-raised desktop, WorkerW: " << workerW << std::endl;
        return workerW;
    }
}

void SetWindowTransparency(HWND hwnd, BYTE alpha = 255)
{
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if ((exStyle & WS_EX_LAYERED) == 0)
    {
        exStyle |= WS_EX_LAYERED;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
    }

    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

void DebugWindowHierarchy()
{
    HWND progman = FindWindowW(L"Progman", nullptr);
    std::cout << "\n=== WINDOW HIERARCHY DEBUG ===" << std::endl;
    std::cout << "Progman: " << progman << std::endl;
    
    if (progman)
    {
        LONG_PTR exStyle = GetWindowLongPtr(progman, GWL_EXSTYLE);
        std::cout << "Progman ExStyle: 0x" << std::hex << exStyle << std::dec << std::endl;
        std::cout << "Has WS_EX_NOREDIRECTIONBITMAP: " << 
            ((exStyle & WS_EX_NOREDIRECTIONBITMAP) ? "YES" : "NO") << std::endl;
        
        // Enumerate ALL children of Progman
        std::cout << "\nProgman children:" << std::endl;
        HWND child = GetWindow(progman, GW_CHILD);
        int childIndex = 0;
        while (child != NULL)
        {
            wchar_t className[256];
            wchar_t windowText[256];
            GetClassNameW(child, className, 256);
            GetWindowTextW(child, windowText, 256);
            
            LONG_PTR childStyle = GetWindowLongPtr(child, GWL_STYLE);
            LONG_PTR childExStyle = GetWindowLongPtr(child, GWL_EXSTYLE);
            
            std::wcout << "  [" << childIndex++ << "] " << child 
                      << " Class: " << className 
                      << " Text: " << windowText
                      << " Style: 0x" << std::hex << childStyle
                      << " ExStyle: 0x" << childExStyle << std::dec << std::endl;
            
            child = GetWindow(child, GW_HWNDNEXT);
        }
    }
    
    std::cout << "\n=== YOUR WINDOW DEBUG ===" << std::endl;
}

void DebugYourWindow(HWND hwnd)
{
    std::cout << "Your window handle: " << hwnd << std::endl;
    
    HWND parent = GetParent(hwnd);
    std::cout << "Parent: " << parent << std::endl;
    
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    
    std::cout << "Style: 0x" << std::hex << style << std::dec << std::endl;
    std::cout << "  WS_CHILD: " << ((style & WS_CHILD) ? "YES" : "NO") << std::endl;
    std::cout << "  WS_VISIBLE: " << ((style & WS_VISIBLE) ? "YES" : "NO") << std::endl;
    
    std::cout << "ExStyle: 0x" << std::hex << exStyle << std::dec << std::endl;
    std::cout << "  WS_EX_LAYERED: " << ((exStyle & WS_EX_LAYERED) ? "YES" : "NO") << std::endl;
    std::cout << "  WS_EX_TOOLWINDOW: " << ((exStyle & WS_EX_TOOLWINDOW) ? "YES" : "NO") << std::endl;
    std::cout << "  WS_EX_NOACTIVATE: " << ((exStyle & WS_EX_NOACTIVATE) ? "YES" : "NO") << std::endl;
    
    RECT rect;
    GetWindowRect(hwnd, &rect);
    std::cout << "Window rect: (" << rect.left << ", " << rect.top 
              << ", " << rect.right << ", " << rect.bottom << ")" << std::endl;
    
    std::cout << "IsWindowVisible: " << (IsWindowVisible(hwnd) ? "YES" : "NO") << std::endl;
    
    // Check layered window attributes
    BYTE alpha;
    COLORREF colorKey;
    DWORD flags;
    if (GetLayeredWindowAttributes(hwnd, &colorKey, &alpha, &flags))
    {
        std::cout << "Layered attributes - Alpha: " << (int)alpha 
                  << " Flags: 0x" << std::hex << flags << std::dec << std::endl;
    }
}

HWND CreateWallpaperContainer(HWND progman, HWND shellDLL_DefView, HWND workerW)
{
    std::cout << "\n=== CREATING WALLPAPER CONTAINER ===" << std::endl;
    
    // Get progman dimensions
    RECT rc;
    if (!GetClientRect(progman, &rc))
    {
        std::cerr << "Failed to get Progman rect: " << GetLastError() << std::endl;
        return NULL;
    }
    
    std::cout << "Creating container with size: " << (rc.right - rc.left) 
              << "x" << (rc.bottom - rc.top) << std::endl;
    std::cout << "Parent (Progman): " << progman << std::endl;
    
    // Clear any previous error
    SetLastError(0);
    
    // Use the built-in STATIC class instead of registering our own
    // This avoids any potential window class registration issues
    HWND container = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"STATIC",  // Use built-in class that we know works
        L"WallpaperContainer",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        progman,
        NULL,
        NULL,  // STATIC class doesn't need instance handle
        NULL
    );
    
    DWORD createError = GetLastError();
    
    if (!container)
    {
        std::cerr << "Failed to create container window. Error: " << createError << std::endl;
        return NULL;
    }
    
    std::cout << "Container created: " << container << std::endl;
    
    // Verify the window was actually created
    if (!IsWindow(container))
    {
        std::cerr << "Container handle is not a valid window!" << std::endl;
        return NULL;
    }
    
    std::cout << "Container is a valid window" << std::endl;
    
    // Set full opacity for the container
    if (!SetLayeredWindowAttributes(container, 0, 255, LWA_ALPHA))
    {
        std::cerr << "Failed to set container opacity: " << GetLastError() << std::endl;
    }
    else
    {
        std::cout << "Container opacity set to 255" << std::endl;
    }
    
    // Position the container between SHELLDLL_DefView and WorkerW
    UINT uFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW;
    if (!SetWindowPos(container, shellDLL_DefView, 0, 0, 0, 0, uFlags))
    {
        std::cerr << "Failed to position container: " << GetLastError() << std::endl;
    }
    else
    {
        std::cout << "Container positioned successfully" << std::endl;
    }
    
    // Show and update
    ShowWindow(container, SW_SHOW);
    UpdateWindow(container);
    
    std::cout << "Container setup complete" << std::endl;
    
    return container;
}

// Add this test function
void TestSimpleWindow()
{
    HWND progman = FindWindowW(L"Progman", nullptr);
    std::cout << "Test: Creating simple child window of Progman" << std::endl;
    
    HWND testWnd = CreateWindowExW(
        0,
        L"STATIC",  // Use built-in STATIC class
        L"Test",
        WS_CHILD | WS_VISIBLE,
        100, 100, 200, 200,
        progman,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );
    
    std::cout << "Test window: " << testWnd << " Error: " << GetLastError() << std::endl;
    
    if (testWnd)
    {
        std::cout << "SUCCESS: Can create child windows of Progman" << std::endl;
        DestroyWindow(testWnd);
    }
    else
    {
        std::cout << "FAILED: Cannot create child windows of Progman" << std::endl;
    }
}

void AttachGLFWWindowToWallpaper(HWND hwnd)
{
    std::cout << "\n=== ATTACHING WINDOW TO DESKTOP ===\n";
    
    // SpawnWorkerW();
    Sleep(200);
    
    HWND progman = FindWindowW(L"Progman", nullptr);
    HWND shellDLL_DefView = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
    HWND workerW = GetWallpaperWorkerW();
    
    std::cout << "Progman: " << progman << std::endl;
    std::cout << "SHELLDLL_DefView: " << shellDLL_DefView << std::endl;
    std::cout << "WorkerW: " << workerW << std::endl;
    
    if (!workerW)
    {
        std::cerr << "Failed to locate WorkerW\n";
        return;
    }
    
    bool isRaisedDesktop = HasExtendedStyle(progman, WS_EX_NOREDIRECTIONBITMAP);
    std::cout << "Is raised desktop: " << (isRaisedDesktop ? "YES" : "NO") << std::endl;
    
    if (isRaisedDesktop)
    {
        std::cout << "\n=== RAISED DESKTOP SETUP ===\n";
        
        // Check and set DPI awareness before changing parent
        // Get DPI awareness of both windows
        std::cout << "Checking DPI awareness...\n";
        
        // Windows 10 1607+ has per-monitor DPI awareness V2
        // We need to ensure our window matches the desktop's DPI awareness
        typedef DPI_AWARENESS_CONTEXT (WINAPI *GetWindowDpiAwarenessContextProc)(HWND);
        typedef BOOL (WINAPI *AreDpiAwarenessContextsEqualProc)(DPI_AWARENESS_CONTEXT, DPI_AWARENESS_CONTEXT);
        
        HMODULE user32 = GetModuleHandleA("user32.dll");
        auto pGetWindowDpiAwarenessContext = (GetWindowDpiAwarenessContextProc)GetProcAddress(user32, "GetWindowDpiAwarenessContext");
        auto pAreDpiAwarenessContextsEqual = (AreDpiAwarenessContextsEqualProc)GetProcAddress(user32, "AreDpiAwarenessContextsEqual");
        
        if (pGetWindowDpiAwarenessContext && pAreDpiAwarenessContextsEqual)
        {
            DPI_AWARENESS_CONTEXT hwndContext = pGetWindowDpiAwarenessContext(hwnd);
            DPI_AWARENESS_CONTEXT progmanContext = pGetWindowDpiAwarenessContext(progman);
            
            if (!pAreDpiAwarenessContextsEqual(hwndContext, progmanContext))
            {
                std::cout << "WARNING: DPI awareness mismatch detected!\n";
                std::cout << "This may cause unexpected behavior.\n";
            }
            else
            {
                std::cout << "DPI awareness contexts match - OK\n";
            }
        }
        
        // Step 1: Modify styles BEFORE SetParent (as per documentation)
        std::cout << "Step 1: Modifying window styles...\n";
        
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        std::cout << "  Original style: 0x" << std::hex << style << std::dec << "\n";
        
        // Clear WS_POPUP and overlapped styles
        style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUP);
        // Set WS_CHILD style (required before SetParent to non-NULL parent)
        style |= WS_CHILD | WS_VISIBLE;
        
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        std::cout << "  New style: 0x" << std::hex << style << std::dec << "\n";
        std::cout << "  WS_POPUP cleared, WS_CHILD set\n";
        
        // Step 2: Set extended styles
        std::cout << "Step 2: Setting extended styles...\n";
        
        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
        exStyle &= ~WS_EX_APPWINDOW;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
        std::cout << "  Extended styles set\n";
        
        // Step 3: Get UI state from parent before SetParent
        std::cout << "Step 3: Synchronizing UI state...\n";
        
        LRESULT progmanUIState = SendMessage(progman, WM_QUERYUISTATE, 0, 0);
        std::cout << "  Progman UI state: 0x" << std::hex << progmanUIState << std::dec << "\n";
        
        // Step 4: Now call SetParent (styles are already set correctly)
        std::cout << "Step 4: Setting parent to Progman...\n";
        
        HWND oldParent = SetParent(hwnd, progman);
        if (!oldParent && GetLastError() != 0)
        {
            std::cerr << "Failed to set parent: " << GetLastError() << std::endl;
            return;
        }
        std::cout << "  Parent set successfully (old parent: " << oldParent << ")\n";
        
        // Step 5: Synchronize UI state after SetParent (as per documentation)
        std::cout << "Step 5: Updating UI state...\n";
        
        SendMessage(hwnd, WM_UPDATEUISTATE, 
                   MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0);
        
        // Apply the parent's UI state to our window
        if (progmanUIState != 0)
        {
            SendMessage(hwnd, WM_CHANGEUISTATE, progmanUIState, 0);
        }
        std::cout << "  UI state synchronized\n";
        
        // Step 6: Position the window
        std::cout << "Step 6: Positioning window...\n";
        
        RECT rc;
        GetClientRect(progman, &rc);
        std::cout << "  Progman client rect: " << rc.right << "x" << rc.bottom << "\n";
        
        // Try positioning below SHELLDLL_DefView
        UINT uFlags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED;
        
        std::cout << "  Attempting to position above WorkerW (below DefView)...\n";
        if (!SetWindowPos(hwnd, workerW,
                         0, 0, rc.right, rc.bottom, uFlags))
        {
            DWORD err = GetLastError();
            std::cerr << "  SetWindowPos failed: " << err << std::endl;
        }
        else
        {
            std::cout << "  Window positioned successfully\n";
        }
        
        // Step 7: Ensure WorkerW stays at bottom
        std::cout << "Step 7: Ensuring WorkerW z-order...\n";
        EnsureWorkerWZOrder(progman, workerW, isRaisedDesktop);
        
        // Step 8: Force redraw
        std::cout << "Step 8: Forcing window update...\n";
        ShowWindow(hwnd, SW_SHOW);
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        
        std::cout << "\n=== SETUP COMPLETE ===\n";
        std::cout << "Expected z-order (top to bottom):\n";
        std::cout << "  1. SHELLDLL_DefView (icons visible)\n";
        std::cout << "  2. Your window (wallpaper)\n";
        std::cout << "  3. WorkerW (bottom)\n";
    }
    else
    {
        // Normal desktop (non-raised)
        std::cout << "\n=== NORMAL DESKTOP SETUP ===\n";
        
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUP);
        style |= WS_CHILD | WS_VISIBLE;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);

        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
        exStyle &= ~WS_EX_APPWINDOW;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

        // Synchronize UI state
        LRESULT workerUIState = SendMessage(workerW, WM_QUERYUISTATE, 0, 0);
        
        SetParent(hwnd, workerW);
        
        SendMessage(hwnd, WM_UPDATEUISTATE, 
                   MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0);
        if (workerUIState != 0)
        {
            SendMessage(hwnd, WM_CHANGEUISTATE, workerUIState, 0);
        }

        RECT rc;
        GetClientRect(workerW, &rc);
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                    SWP_NOACTIVATE | SWP_SHOWWINDOW);
        
        std::cout << "Setup complete\n";
    }
}