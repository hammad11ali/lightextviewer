#pragma once

#include <string>
#include <vector>
#include "types.h"

namespace editor {

/**
 * UTF-8/UTF-16 codec for Windows interop
 * Handles conversion between UTF-8 (internal storage) and UTF-16 (Windows APIs)
 */
class UtfCodec {
public:
    /**
     * Convert UTF-8 string to UTF-16 wide string
     * @param utf8 Input UTF-8 encoded string
     * @return UtfConversionResult with UTF-16 string
     */
    static UtfConversionResult utf8ToUtf16(const std::string& utf8);
    
    /**
     * Convert UTF-16 wide string to UTF-8 string
     * @param utf16 Input UTF-16 encoded wide string
     * @return UtfConversionResult with UTF-8 string
     */
    static UtfConversionResult utf16ToUtf8(const std::wstring& utf16);
    
    /**
     * Convert a range of UTF-8 bytes to UTF-16
     * @param data Pointer to UTF-8 data
     * @param length Number of bytes to convert
     * @return UtfConversionResult with UTF-16 string
     */
    static UtfConversionResult utf8RangeToUtf16(const char* data, size_t length);
    
    /**
     * Get the byte length of a UTF-8 character at the given position
     * @param data Pointer to UTF-8 data
     * @param length Total length of data
     * @param pos Position to check
     * @return Number of bytes in the UTF-8 character (1-4), or 0 if invalid
     */
    static int getUtf8CharLength(const char* data, size_t length, size_t pos);
    
    /**
     * Check if a byte is a UTF-8 continuation byte (10xxxxxx)
     * @param byte The byte to check
     * @return true if continuation byte
     */
    static bool isUtf8ContinuationByte(char byte);
    
    /**
     * Validate UTF-8 data
     * @param data Pointer to UTF-8 data
     * @param length Length of data
     * @return true if valid UTF-8, false otherwise
     */
    static bool isValidUtf8(const char* data, size_t length);
};

} // namespace editor
