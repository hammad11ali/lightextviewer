#pragma once

#include <string>
#include <vector>
#include <memory>
#include "types.h"

namespace editor {

/**
 * Piece Table data structure for efficient text editing
 * Uses two buffers: original (read-only) and add (append-only)
 * Supports large files through memory-mapped file integration
 */
class PieceTree {
public:
    /**
     * A piece represents a segment of text from either the original or add buffer
     */
    struct Piece {
        size_t bufferOffset = 0;   // Offset in the source buffer
        size_t length = 0;          // Length of this segment
        bool isFromOriginal = true; // True if from original buffer, false if from add buffer
        
        Piece() = default;
        Piece(size_t offset, size_t len, bool fromOriginal)
            : bufferOffset(offset), length(len), isFromOriginal(fromOriginal) {}
    };
    
    PieceTree();
    ~PieceTree();
    
    /**
     * Initialize with file content (original buffer)
     * @param data Pointer to UTF-8 file data
     * @param length Length of data in bytes
     */
    void initialize(const char* data, size_t length);
    
    /**
     * Insert text at the given position
     * @param pos Byte offset where to insert
     * @param text Text to insert (UTF-8)
     * @return true if successful
     */
    bool insert(size_t pos, const std::string& text);
    
    /**
     * Delete text in the given range
     * @param start Start byte offset
     * @param end End byte offset (exclusive)
     * @return true if successful
     */
    bool remove(size_t start, size_t end);
    
    /**
     * Get text in the given range
     * @param start Start byte offset
     * @param length Number of bytes to retrieve
     * @return UTF-8 string
     */
    std::string getText(size_t start, size_t length) const;
    
    /**
     * Get a single character at the given position
     * @param pos Byte offset
     * @return UTF-8 string containing the character
     */
    std::string getCharAt(size_t pos) const;
    
    /**
     * Get the total length of the document in bytes
     * @return Total byte count
     */
    size_t getTotalLength() const;
    
    /**
     * Get line count
     * @return Number of lines in the document
     */
    size_t getLineCount() const;
    
    /**
     * Convert byte offset to line/column position
     * @param byteOffset Byte offset in document
     * @return TextPosition with line and column
     */
    TextPosition offsetToPosition(size_t byteOffset) const;
    
    /**
     * Convert line/column position to byte offset
     * @param position Line/column position
     * @return Byte offset in document
     */
    size_t positionToOffset(const TextPosition& position) const;
    
    /**
     * Get the start byte offset of a specific line
     * @param line Line number (0-based)
     * @return Byte offset of line start, or INVALID_POSITION if line doesn't exist
     */
    size_t getLineStartOffset(size_t line) const;
    
    /**
     * Get the length of a specific line in bytes (excluding newline)
     * @param line Line number (0-based)
     * @return Line length in bytes
     */
    size_t getLineLength(size_t line) const;
    
    /**
     * Get visible text for rendering
     * @param startLine First visible line
     * @param endLine Last visible line
     * @return Vector of UTF-8 strings, one per line
     */
    std::vector<std::string> getVisibleLines(size_t startLine, size_t endLine) const;
    
    /**
     * Check if document has been modified
     * @return true if modified
     */
    bool isModified() const { return m_modified; }
    
    /**
     * Get pointer to original buffer data
     * @return Pointer to original UTF-8 data
     */
    const char* getOriginalData() const { return m_originalBuffer; }
    
    /**
     * Get pointer to add buffer data
     * @return Pointer to add buffer UTF-8 data
     */
    const char* getAddBufferData() const { return m_addBuffer.data(); }
    
    /**
     * Get current document as contiguous string (for saving)
     * @return Complete document as UTF-8 string
     */
    std::string getFullText() const;
    
private:
    /**
     * Find which piece contains a given offset
     * @param offset Byte offset to find
     * @param outPieceIndex Output: index of the piece
     * @param outOffsetInPiece Output: offset within that piece
     * @return true if found
     */
    bool findPieceAt(size_t offset, size_t& outPieceIndex, size_t& outOffsetInPiece) const;
    
    /**
     * Split a piece at the given offset within the piece
     * @param pieceIndex Index of piece to split
     * @param offsetInPiece Offset within the piece to split at
     */
    void splitPiece(size_t pieceIndex, size_t offsetInPiece);
    
    /**
     * Rebuild the line index after modifications
     */
    void rebuildLineIndex();
    
    const char* m_originalBuffer = nullptr;  // Original file data (not owned)
    size_t m_originalLength = 0;              // Length of original buffer
    
    std::string m_addBuffer;                  // Append-only buffer for edits
    std::vector<Piece> m_pieces;              // Sequence of pieces
    
    std::vector<size_t> m_lineOffsets;        // Byte offset of each line start
    size_t m_totalLength = 0;                 // Current total document length
    bool m_modified = false;                  // Whether document has been edited
};

} // namespace editor
