#include "clipboard_handler.h"
#include "utf_codec.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace editor {

bool ClipboardHandler::copy(const std::string& text) {
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    EmptyClipboard();
    
    // Convert UTF-8 to UTF-16 for Windows clipboard
    UtfConversionResult result = UtfCodec::utf8ToUtf16(text);
    if (!result.success || result.utf16.empty()) {
        CloseClipboard();
        return false;
    }
    
    // Allocate global memory for clipboard
    size_t bytesNeeded = (result.utf16.length() + 1) * sizeof(wchar_t);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, bytesNeeded);
    
    if (!hGlobal) {
        CloseClipboard();
        return false;
    }
    
    wchar_t* buffer = static_cast<wchar_t*>(GlobalLock(hGlobal));
    if (!buffer) {
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }
    
    // Copy text to buffer
    wcscpy_s(buffer, result.utf16.length() + 1, result.utf16.c_str());
    
    GlobalUnlock(hGlobal);
    
    // Set clipboard data
    if (!SetClipboardData(CF_UNICODETEXT, hGlobal)) {
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }
    
    CloseClipboard();
    return true;
#else
    // Non-Windows placeholder
    return false;
#endif
}

std::string ClipboardHandler::paste() {
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) {
        return "";
    }
    
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        CloseClipboard();
        return "";
    }
    
    const wchar_t* wideText = static_cast<const wchar_t*>(GlobalLock(hData));
    if (!wideText) {
        CloseClipboard();
        return "";
    }
    
    // Convert UTF-16 to UTF-8
    UtfConversionResult result = UtfCodec::utf16ToUtf8(std::wstring(wideText));
    
    GlobalUnlock(hData);
    CloseClipboard();
    
    if (!result.success) {
        return "";
    }
    
    return result.utf8;
#else
    // Non-Windows placeholder
    return "";
#endif
}

bool ClipboardHandler::hasText() const {
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    bool hasText = IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
    
    CloseClipboard();
    return hasText;
#else
    return false;
#endif
}

bool ClipboardHandler::clear() {
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    bool success = EmptyClipboard() != 0;
    
    CloseClipboard();
    return success;
#else
    return false;
#endif
}

} // namespace editor
