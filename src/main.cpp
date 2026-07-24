/**
 * Lightweight Large-File Text Editor for Windows
 * 
 * Main entry point
 */

#ifdef _WIN32
#include <windows.h>
#else
// Non-Windows type definitions
typedef void* HINSTANCE;
typedef wchar_t* LPWSTR;
#define WINAPI
#endif

#include "window.h"

#ifdef _WIN32
int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow)
{
    (void)hPrevInstance;
    (void)nCmdShow;
    
    // Create main window
    editor::Window mainWindow;
    
    if (!mainWindow.create(hInstance, L"Lightweight Editor")) {
        MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // If command line has a file path, try to open it
    if (lpCmdLine && lpCmdLine[0] != L'\0') {
        mainWindow.loadFile(lpCmdLine);
    }
    
    // Run message loop
    return mainWindow.runMessageLoop();
}
#else
// Placeholder for non-Windows compilation testing
int main() {
    return 0;
}
#endif
