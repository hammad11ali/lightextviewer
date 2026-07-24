#pragma once

#include <string>
#include <vector>
#include "types.h"

namespace editor {

/**
 * File handler for opening and saving files
 * Supports memory-mapped files for large file handling
 */
class FileHandler {
public:
    FileHandler() = default;
    ~FileHandler() = default;
    
    /**
     * Open a file and read its contents
     * @param path Path to the file (UTF-16 wide string for Windows)
     * @return FileInfo with file details
     */
    FileInfo openFile(const std::wstring& path);
    
    /**
     * Save content to a file
     * @param path Path to the file (UTF-16 wide string for Windows)
     * @param content Content to save (UTF-8)
     * @param preserveBom Whether to preserve UTF-8 BOM if present
     * @return true if successful
     */
    bool saveFile(const std::wstring& path, const std::string& content, bool preserveBom = false);
    
    /**
     * Check if a file exists
     * @param path Path to check
     * @return true if file exists
     */
    bool fileExists(const std::wstring& path) const;
    
    /**
     * Get file size
     * @param path Path to the file
     * @return File size in bytes, or 0 if file doesn't exist
     */
    size_t getFileSize(const std::wstring& path) const;
    
    /**
     * Check if file is writable
     * @param path Path to check
     * @return true if file can be written to
     */
    bool isWritable(const std::wstring& path) const;
    
    /**
     * Get the directory portion of a path
     * @param path Full file path
     * @return Directory path
     */
    std::wstring getDirectory(const std::wstring& path) const;
    
    /**
     * Get the filename portion of a path
     * @param path Full file path
     * @return Filename
     */
    std::wstring getFilename(const std::wstring& path) const;
    
private:
    /**
     * Detect if file has UTF-8 BOM
     * @param data Pointer to file data
     * @param length Length of data
     * @return true if BOM detected
     */
    bool hasUtf8Bom(const char* data, size_t length) const;
};

} // namespace editor
