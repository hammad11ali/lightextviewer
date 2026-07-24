#pragma once

#include <string>
#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#else
// Non-Windows type definitions for compilation testing
typedef int LRESULT;
typedef unsigned int UINT;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
typedef ULONG_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef void* HWND;
typedef void* HINSTANCE;
#define CALLBACK
#define WINAPI

// Windows constants
#define GWLP_USERDATA (-21)
#define WM_NCCREATE 0x0081
#endif

namespace editor {

class PieceTree;
class SearchEngine;
class UndoManager;
class Renderer;
class ClipboardHandler;
class FileHandler;

/**
 * Main window class for the editor
 * Handles Win32 window creation, message loop, and user input
 */
class Window {
public:
    Window();
    ~Window();
    
    /**
     * Create the main window
     * @param instance HINSTANCE for the application
     * @param title Window title
     * @return true if successful
     */
    bool create(void* instance, const std::wstring& title);
    
    /**
     * Run the message loop
     * @return Exit code
     */
    int runMessageLoop();
    
    /**
     * Close the window
     */
    void close();
    
    /**
     * Get the window handle
     * @return HWND
     */
    void* getHwnd() const { return m_hwnd; }
    
    /**
     * Load a file into the editor
     * @param path Path to the file
     * @return true if successful
     */
    bool loadFile(const std::wstring& path);
    
    /**
     * Save the current file
     * @return true if successful
     */
    bool saveFile();
    
private:
    /**
     * Process window messages
     * @param msg Message ID
     * @param wParam Message parameter
     * @param lParam Message parameter
     * @return Result
     */
    LRESULT processMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    
    /**
     * Handle keyboard input
     * @param key Virtual key code
     * @param ctrl Ctrl key state
     * @param shift Shift key state
     */
    void handleKeyDown(int key, bool ctrl, bool shift);
    
    /**
     * Update window title with file info
     */
    void updateTitle();
    
    /**
     * Show search bar
     */
    void showSearchBar();
    
    /**
     * Show replace bar
     */
    void showReplaceBar();
    
    // Static window procedure
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    void* m_hwnd = nullptr;           // Window handle
    void* m_instance = nullptr;       // Application instance
    
    // Editor components
    PieceTree* m_document = nullptr;
    SearchEngine* m_searchEngine = nullptr;
    UndoManager* m_undoManager = nullptr;
    Renderer* m_renderer = nullptr;
    ClipboardHandler* m_clipboard = nullptr;
    FileHandler* m_fileHandler = nullptr;
    
    // State
    std::wstring m_currentFilePath;
    bool m_hasBom = false;
    bool m_isModified = false;
    
    // Caret position
    size_t m_caretLine = 0;
    size_t m_caretColumn = 0;
    
    // Selection
    size_t m_selectionStartLine = 0;
    size_t m_selectionStartColumn = 0;
    size_t m_selectionEndLine = 0;
    size_t m_selectionEndColumn = 0;
    bool m_hasSelection = false;
    
    // Search state
    bool m_searchVisible = false;
    bool m_replaceVisible = false;
    std::string m_searchQuery;
    bool m_caseSensitive = true;
};

} // namespace editor
