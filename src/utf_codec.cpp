#include "utf_codec.h"
#include <cstring>

namespace editor {

UtfConversionResult UtfCodec::utf8ToUtf16(const std::string& utf8) {
    UtfConversionResult result;
    
    if (utf8.empty()) {
        result.utf16 = L"";
        return result;
    }
    
    // Calculate required length
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), 
                                   static_cast<int>(utf8.length()), nullptr, 0);
    
    if (len <= 0) {
        result.success = false;
        result.error = "Failed to convert UTF-8 to UTF-16";
        return result;
    }
    
    result.utf16.resize(len);
    int convLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(),
                                       static_cast<int>(utf8.length()),
                                       result.utf16.data(), len);
    
    if (convLen != len) {
        result.success = false;
        result.error = "UTF-8 to UTF-16 conversion error";
        return result;
    }
    
    return result;
}

UtfConversionResult UtfCodec::utf16ToUtf8(const std::wstring& utf16) {
    UtfConversionResult result;
    
    if (utf16.empty()) {
        result.utf8 = "";
        return result;
    }
    
    // Calculate required length
    int len = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(),
                                   static_cast<int>(utf16.length()), nullptr, 0,
                                   nullptr, nullptr);
    
    if (len <= 0) {
        result.success = false;
        result.error = "Failed to convert UTF-16 to UTF-8";
        return result;
    }
    
    result.utf8.resize(len);
    int convLen = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(),
                                       static_cast<int>(utf16.length()),
                                       result.utf8.data(), len,
                                       nullptr, nullptr);
    
    if (convLen != len) {
        result.success = false;
        result.error = "UTF-16 to UTF-8 conversion error";
        return result;
    }
    
    return result;
}

UtfConversionResult UtfCodec::utf8RangeToUtf16(const char* data, size_t length) {
    UtfConversionResult result;
    
    if (!data || length == 0) {
        result.utf16 = L"";
        return result;
    }
    
    int len = MultiByteToWideChar(CP_UTF8, 0, data, 
                                   static_cast<int>(length), nullptr, 0);
    
    if (len <= 0) {
        result.success = false;
        result.error = "Failed to convert UTF-8 range to UTF-16";
        return result;
    }
    
    result.utf16.resize(len);
    int convLen = MultiByteToWideChar(CP_UTF8, 0, data,
                                       static_cast<int>(length),
                                       result.utf16.data(), len);
    
    if (convLen != len) {
        result.success = false;
        result.error = "UTF-8 range conversion error";
        return result;
    }
    
    return result;
}

int UtfCodec::getUtf8CharLength(const char* data, size_t length, size_t pos) {
    if (!data || pos >= length) {
        return 0;
    }
    
    unsigned char byte = static_cast<unsigned char>(data[pos]);
    
    // ASCII (0xxxxxxx)
    if ((byte & 0x80) == 0) {
        return 1;
    }
    // Invalid (10xxxxxx - continuation byte as start)
    else if ((byte & 0xC0) == 0x80) {
        return 0;
    }
    // 2-byte sequence (110xxxxx)
    else if ((byte & 0xE0) == 0xC0) {
        return (pos + 1 < length) ? 2 : 0;
    }
    // 3-byte sequence (1110xxxx)
    else if ((byte & 0xF0) == 0xE0) {
        return (pos + 2 < length) ? 3 : 0;
    }
    // 4-byte sequence (11110xxx)
    else if ((byte & 0xF8) == 0xF0) {
        return (pos + 3 < length) ? 4 : 0;
    }
    // Invalid start byte
    else {
        return 0;
    }
}

bool UtfCodec::isUtf8ContinuationByte(char byte) {
    return (static_cast<unsigned char>(byte) & 0xC0) == 0x80;
}

bool UtfCodec::isValidUtf8(const char* data, size_t length) {
    if (!data) {
        return false;
    }
    
    size_t pos = 0;
    while (pos < length) {
        int charLen = getUtf8CharLength(data, length, pos);
        if (charLen == 0) {
            return false;
        }
        
        // Verify continuation bytes
        for (int i = 1; i < charLen; ++i) {
            if (!isUtf8ContinuationByte(data[pos + i])) {
                return false;
            }
        }
        
        pos += charLen;
    }
    
    return true;
}

} // namespace editor
