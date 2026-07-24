#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace editor {

// Basic types for text positions
struct TextPosition {
    size_t line = 0;
    size_t column = 0;
    
    bool operator==(const TextPosition& other) const = default;
    bool operator!=(const TextPosition& other) const = default;
    bool operator<(const TextPosition& other) const {
        if (line != other.line) return line < other.line;
        return column < other.column;
    }
    bool operator<=(const TextPosition& other) const {
        return !(*this > other);
    }
    bool operator>(const TextPosition& other) const {
        return other < *this;
    }
};

struct TextSelection {
    TextPosition start;
    TextPosition end;
    
    bool isValid() const { return start <= end; }
    bool isEmpty() const { return start == end; }
};

// UTF-8/UTF-16 conversion result
struct UtfConversionResult {
    std::string utf8;
    std::wstring utf16;
    bool success = true;
    std::string error;
};

// Search result
struct SearchResult {
    size_t position = 0;  // Byte offset in original file
    size_t length = 0;
    TextPosition textPos; // Line/column position
};

// File information
struct FileInfo {
    std::wstring path;
    size_t size = 0;
    bool hasBom = false;
    bool isModified = false;
    bool isValid = false;
    std::string error;
    std::string content;  // File content in UTF-8
};

// Constants
constexpr size_t INVALID_POSITION = static_cast<size_t>(-1);
constexpr size_t CHUNK_SIZE = 65536;  // 64KB chunks for file reading
constexpr size_t MAX_SEARCH_RESULTS = 100000;  // Cap for very large files

} // namespace editor
