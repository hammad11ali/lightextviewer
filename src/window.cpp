#include "window.h"
#include "piece_tree.h"
#include "search_engine.h"
#include "undo_manager.h"
#include "renderer.h"
#include "clipboard_handler.h"
#include "file_handler.h"
#include "utf_codec.h"

#ifdef _WIN32
#include <windowsx.h>
#include <commdlg.h>
#endif

namespace editor {

Window::Window() {
    m_document = new PieceTree();
    m_searchEngine = new SearchEngine();
    m_undoManager = new UndoManager();
    m_renderer = new Renderer();
    m_clipboard = new ClipboardHandler();
    m_fileHandler = new FileHandler();
}

Window::~Window() {
    delete m_document;
    delete m_searchEngine;
    delete m_undoManager;
    delete m_renderer;
    delete m_clipboard;
    delete m_fileHandler;
}

bool Window::create(void* instance, const std::wstring& title) {
    m_instance = instance;
    
#ifdef _WIN32
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = windowProc;
    wc.hInstance = static_cast<HINSTANCE>(instance);
    wc.hCursor = LoadCursor(nullptr, IDC_IBEAM);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = L"LightweightEditor";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    
    if (!RegisterClassExW(&wc)) {
        return false;
    }
    
    // Create window
    m_hwnd = CreateWindowExW(
        0,
        L"LightweightEditor",
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1024, 768,
        nullptr, nullptr,
        static_cast<HINSTANCE>(instance),
        this
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    // Set user data for window procedure
    SetWindowLongPtrW(static_cast<HWND>(m_hwnd), GWLP_USERDATA, 
                      reinterpret_cast<LONG_PTR>(this));
    
    // Initialize renderer
    m_renderer->initialize(m_hwnd);
    
    ShowWindow(static_cast<HWND>(m_hwnd), SW_SHOW);
    UpdateWindow(static_cast<HWND>(m_hwnd));
    
    return true;
#else
    (void)instance;
    (void)title;
    return false;
#endif
}

int Window::runMessageLoop() {
#ifdef _WIN32
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
#else
    return 0;
#endif
}

void Window::close() {
#ifdef _WIN32
    if (m_hwnd) {
        DestroyWindow(static_cast<HWND>(m_hwnd));
    }
#endif
}

bool Window::loadFile(const std::wstring& path) {
    FileInfo info = m_fileHandler->openFile(path);
    if (!info.isValid) {
        return false;
    }
    
    m_currentFilePath = path;
    m_hasBom = info.hasBom;
    m_isModified = false;
    
    // Load actual file content
    m_document->initialize(info.content.data(), info.content.length());
    
    // Reset caret position
    m_caretLine = 0;
    m_caretColumn = 0;
    m_hasSelection = false;
    
    updateTitle();
    
    // Force repaint
#ifdef _WIN32
    InvalidateRect(static_cast<HWND>(m_hwnd), nullptr, TRUE);
#endif
    
    return true;
}

bool Window::saveFile() {
    if (m_currentFilePath.empty()) {
        return false;
    }
    
    std::string content = m_document->getFullText();
    bool success = m_fileHandler->saveFile(m_currentFilePath, content, m_hasBom);
    
    if (success) {
        m_isModified = false;
        updateTitle();
    }
    
    return success;
}

LRESULT CALLBACK Window::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Get window instance from user data
    Window* window = reinterpret_cast<Window*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    
    // Handle WM_NCCREATE to set up user data
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = reinterpret_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 
                         reinterpret_cast<LONG_PTR>(window));
        window->m_hwnd = hwnd;
    }
    
    if (window) {
        return window->processMessage(msg, wParam, lParam);
    }
    
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Window::processMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
#ifdef _WIN32
    switch (msg) {
        case WM_COMMAND:
            // Handle menu commands
            break;
            
        case WM_KEYDOWN:
            {
                bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                handleKeyDown(static_cast<int>(wParam), ctrl, shift);
            }
            break;
            
        case WM_CHAR:
            // Handle text input
            if (wParam >= 32 && wParam != 127) {
                // Convert to UTF-8 and insert
                wchar_t wideChar = static_cast<wchar_t>(wParam);
                auto result = UtfCodec::utf16ToUtf8(std::wstring(1, wideChar));
                if (result.success) {
                    size_t caretOffset = m_document->positionToOffset(
                        TextPosition{m_caretLine, m_caretColumn});
                    m_document->insert(caretOffset, result.utf8);
                    m_caretColumn++;
                    InvalidateRect(static_cast<HWND>(m_hwnd), nullptr, TRUE);
                }
            }
            break;
            
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(static_cast<HWND>(m_hwnd), &ps);
                
                // Get visible lines and render
                size_t visibleLines = m_renderer->getVisibleLineCount();
                auto lines = m_document->getVisibleLines(
                    m_renderer->getScrollPosition(),
                    m_renderer->getScrollPosition() + visibleLines);
                
                m_renderer->render(lines, m_renderer->getScrollPosition(),
                                  m_caretLine, m_caretColumn,
                                  m_selectionStartLine, m_selectionEndLine);
                
                EndPaint(static_cast<HWND>(m_hwnd), &ps);
            }
            break;
            
        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                m_renderer->onResize(width, height);
            }
            break;
            
        case WM_VSCROLL:
            // Handle vertical scrolling
            break;
            
        case WM_HSCROLL:
            // Handle horizontal scrolling
            break;
            
        case WM_CLOSE:
            close();
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProcW(static_cast<HWND>(m_hwnd), msg, wParam, lParam);
    }
#endif
    return 0;
}

void Window::handleKeyDown(int key, bool ctrl, bool shift) {
    (void)shift;
    
#ifdef _WIN32
    if (ctrl) {
        switch (key) {
            case 'O':  // Ctrl+O - Open file
                {
                    OPENFILENAMEW ofn = {};
                    wchar_t szFile[MAX_PATH] = L"";
                    
                    ofn.lStructSize = sizeof(OPENFILENAMEW);
                    ofn.hwndOwner = static_cast<HWND>(m_hwnd);
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0";
                    ofn.nFilterIndex = 1;
                    ofn.lpstrFileTitle = nullptr;
                    ofn.nMaxFileTitle = 0;
                    ofn.lpstrInitialDir = nullptr;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    
                    if (GetOpenFileNameW(&ofn)) {
                        loadFile(szFile);
                    }
                }
                break;
                
            case 'S':  // Ctrl+S - Save
                saveFile();
                break;
                
            case 'F':  // Ctrl+F - Find
                showSearchBar();
                break;
                
            case 'H':  // Ctrl+H - Replace
                showReplaceBar();
                break;
                
            case 'Z':  // Ctrl+Z - Undo
                m_undoManager->undo();
                break;
                
            case 'Y':  // Ctrl+Y - Redo
                m_undoManager->redo();
                break;
                
            case 'C':  // Ctrl+C - Copy
                if (m_hasSelection) {
                    size_t start = m_document->positionToOffset(
                        TextPosition{m_selectionStartLine, m_selectionStartColumn});
                    size_t end = m_document->positionToOffset(
                        TextPosition{m_selectionEndLine, m_selectionEndColumn});
                    std::string selected = m_document->getText(start, end - start);
                    m_clipboard->copy(selected);
                }
                break;
                
            case 'V':  // Ctrl+V - Paste
                {
                    std::string clipboardText = m_clipboard->paste();
                    if (!clipboardText.empty()) {
                        size_t caretOffset = m_document->positionToOffset(
                            TextPosition{m_caretLine, m_caretColumn});
                        m_document->insert(caretOffset, clipboardText);
                        // Update caret position
                        auto newPos = m_document->offsetToPosition(
                            caretOffset + clipboardText.length());
                        m_caretLine = newPos.line;
                        m_caretColumn = newPos.column;
                    }
                }
                break;
        }
    } else {
        switch (key) {
            case VK_LEFT:
                if (m_caretColumn > 0) {
                    m_caretColumn--;
                }
                break;
                
            case VK_RIGHT:
                m_caretColumn++;
                break;
                
            case VK_UP:
                if (m_caretLine > 0) {
                    m_caretLine--;
                }
                break;
                
            case VK_DOWN:
                m_caretLine++;
                break;
                
            case VK_HOME:
                m_caretColumn = 0;
                break;
                
            case VK_END:
                {
                    size_t lineLen = m_document->getLineLength(m_caretLine);
                    m_caretColumn = lineLen;
                }
                break;
                
            case VK_DELETE:
                {
                    size_t offset = m_document->positionToOffset(
                        TextPosition{m_caretLine, m_caretColumn});
                    if (offset < m_document->getTotalLength()) {
                        m_document->remove(offset, offset + 1);
                    }
                }
                break;
                
            case VK_BACK:
                {
                    if (m_caretColumn > 0) {
                        size_t offset = m_document->positionToOffset(
                            TextPosition{m_caretLine, m_caretColumn});
                        m_document->remove(offset - 1, offset);
                        m_caretColumn--;
                    } else if (m_caretLine > 0) {
                        // Delete previous line's newline
                        m_caretLine--;
                        size_t prevLineLen = m_document->getLineLength(m_caretLine);
                        size_t offset = m_document->positionToOffset(
                            TextPosition{m_caretLine, prevLineLen});
                        m_document->remove(offset, offset + 1);
                    }
                }
                break;
                
            case VK_ESCAPE:
                m_searchVisible = false;
                m_replaceVisible = false;
                break;
        }
    }
#else
    (void)key;
    (void)ctrl;
#endif
}

void Window::updateTitle() {
    // Window title update would happen here
}

void Window::showSearchBar() {
    m_searchVisible = true;
}

void Window::showReplaceBar() {
    m_replaceVisible = true;
}

} // namespace editor
