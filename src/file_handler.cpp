#include "file_handler.h"

#ifdef _WIN32
#include <windows.h>
#else
// Placeholder for non-Windows (for compilation testing)
#include <sys/stat.h>
#include <fstream>
#endif

namespace editor {

FileInfo FileHandler::openFile(const std::wstring& path) {
    FileInfo info;
    info.path = path;
    info.isValid = false;
    
#ifdef _WIN32
    // Windows implementation using Win32 API
    HANDLE hFile = CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        info.error = "Failed to open file";
        return info;
    }
    
    // Get file size
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        info.error = "Failed to get file size";
        return info;
    }
    
    info.size = static_cast<size_t>(fileSize.QuadPart);
    
    // For now, we just return file info without loading content
    // The actual content loading will be done via memory mapping in the editor
    info.isValid = true;
    
    // Check for BOM by reading first few bytes
    char bom[4];
    DWORD bytesRead = 0;
    if (ReadFile(hFile, bom, 3, &bytesRead, nullptr) && bytesRead >= 3) {
        info.hasBom = (static_cast<unsigned char>(bom[0]) == 0xEF &&
                       static_cast<unsigned char>(bom[1]) == 0xBB &&
                       static_cast<unsigned char>(bom[2]) == 0xBF);
    }
    
    CloseHandle(hFile);
#else
    // Non-Windows fallback (for compilation testing)
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        info.error = "Failed to access file";
        return info;
    }
    
    info.size = static_cast<size_t>(st.st_size);
    info.isValid = true;
    info.hasBom = false;
#endif
    
    return info;
}

bool FileHandler::saveFile(const std::wstring& path, const std::string& content, 
                           bool preserveBom) {
#ifdef _WIN32
    // Windows implementation using Win32 API
    HANDLE hFile = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,  // No sharing during write
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    // Write BOM if requested
    if (preserveBom) {
        const char bom[] = "\xEF\xBB\xBF";
        DWORD bytesWritten = 0;
        WriteFile(hFile, bom, 3, &bytesWritten, nullptr);
    }
    
    // Write content
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(
        hFile,
        content.data(),
        static_cast<DWORD>(content.length()),
        &bytesWritten,
        nullptr
    );
    
    CloseHandle(hFile);
    
    return (result != FALSE) && (bytesWritten == content.length());
#else
    // Non-Windows fallback
    std::ofstream ofs(path.c_str(), std::ios::binary);
    if (!ofs) {
        return false;
    }
    
    if (preserveBom) {
        ofs.write("\xEF\xBB\xBF", 3);
    }
    
    ofs.write(content.data(), static_cast<std::streamsize>(content.length()));
    return ofs.good();
#endif
}

bool FileHandler::fileExists(const std::wstring& path) const {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesW(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && 
           !(attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return (stat(path.c_str(), &st) == 0) && S_ISREG(st.st_mode);
#endif
}

size_t FileHandler::getFileSize(const std::wstring& path) const {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fad)) {
        return 0;
    }
    LARGE_INTEGER size;
    size.LowPart = fad.nFileSizeLow;
    size.HighPart = fad.nFileSizeHigh;
    return static_cast<size_t>(size.QuadPart);
#else
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<size_t>(st.st_size);
#endif
}

bool FileHandler::isWritable(const std::wstring& path) const {
#ifdef _WIN32
    // Check if file exists and is not read-only
    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        return false;
    }
    
    // Try to open for writing
    HANDLE hFile = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    CloseHandle(hFile);
    return true;
#else
    return access(path.c_str(), W_OK) == 0;
#endif
}

std::wstring FileHandler::getDirectory(const std::wstring& path) const {
    size_t pos = path.find_last_of(L"/\\");
    if (pos == std::wstring::npos) {
        return L"";
    }
    return path.substr(0, pos);
}

std::wstring FileHandler::getFilename(const std::wstring& path) const {
    size_t pos = path.find_last_of(L"/\\");
    if (pos == std::wstring::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

bool FileHandler::hasUtf8Bom(const char* data, size_t length) const {
    if (length < 3) {
        return false;
    }
    return (static_cast<unsigned char>(data[0]) == 0xEF &&
            static_cast<unsigned char>(data[1]) == 0xBB &&
            static_cast<unsigned char>(data[2]) == 0xBF);
}

} // namespace editor
