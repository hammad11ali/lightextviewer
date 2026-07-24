#pragma once

#include <string>
#include "types.h"

namespace editor {

/**
 * Clipboard handler for Windows clipboard operations
 */
class ClipboardHandler {
public:
    ClipboardHandler() = default;
    ~ClipboardHandler() = default;
    
    /**
     * Copy text to clipboard
     * @param text UTF-8 text to copy
     * @return true if successful
     */
    bool copy(const std::string& text);
    
    /**
     * Paste text from clipboard
     * @return UTF-8 text from clipboard, or empty string if unavailable
     */
    std::string paste();
    
    /**
     * Check if clipboard contains text
     * @return true if clipboard has text content
     */
    bool hasText() const;
    
    /**
     * Clear clipboard
     * @return true if successful
     */
    bool clear();
};

} // namespace editor
