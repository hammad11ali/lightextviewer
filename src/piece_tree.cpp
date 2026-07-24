#include "piece_tree.h"
#include "utf_codec.h"
#include <cstring>
#include <algorithm>

namespace editor {

PieceTree::PieceTree() = default;
PieceTree::~PieceTree() = default;

void PieceTree::initialize(const char* data, size_t length) {
    m_originalBuffer = data;
    m_originalLength = length;
    m_addBuffer.clear();
    m_pieces.clear();
    m_modified = false;
    
    // Create initial piece covering the entire original buffer
    if (length > 0) {
        m_pieces.emplace_back(0, length, true);
        m_totalLength = length;
    } else {
        m_totalLength = 0;
    }
    
    rebuildLineIndex();
}

bool PieceTree::insert(size_t pos, const std::string& text) {
    if (text.empty()) {
        return true;
    }
    
    if (pos > m_totalLength) {
        pos = m_totalLength;
    }
    
    // Add text to add buffer
    size_t addOffset = m_addBuffer.length();
    m_addBuffer += text;
    size_t textLength = text.length();
    
    // Find where to insert in piece list
    size_t pieceIndex = 0;
    size_t offsetInPiece = 0;
    
    if (!findPieceAt(pos, pieceIndex, offsetInPiece)) {
        // Position is at the end, append new piece
        m_pieces.emplace_back(addOffset, textLength, false);
        m_totalLength += textLength;
        m_modified = true;
        rebuildLineIndex();
        return true;
    }
    
    // Split the piece at the insertion point if needed
    if (offsetInPiece > 0 && offsetInPiece < m_pieces[pieceIndex].length) {
        splitPiece(pieceIndex, offsetInPiece);
        pieceIndex++;  // New piece is after the split
    }
    
    // Insert new piece
    Piece newPiece(addOffset, textLength, false);
    m_pieces.insert(m_pieces.begin() + pieceIndex, newPiece);
    
    m_totalLength += textLength;
    m_modified = true;
    rebuildLineIndex();
    
    return true;
}

bool PieceTree::remove(size_t start, size_t end) {
    if (start >= end || start >= m_totalLength) {
        return false;
    }
    
    if (end > m_totalLength) {
        end = m_totalLength;
    }
    
    size_t removeLength = end - start;
    
    // Find the pieces affected by the removal
    size_t startPieceIndex = 0;
    size_t startOffsetInPiece = 0;
    size_t endPieceIndex = 0;
    size_t endOffsetInPiece = 0;
    
    if (!findPieceAt(start, startPieceIndex, startOffsetInPiece)) {
        return false;
    }
    
    if (!findPieceAt(end, endPieceIndex, endOffsetInPiece)) {
        // If end is at document end, set to last piece
        if (end == m_totalLength + removeLength) {  // Account for not-yet-removed length
            endPieceIndex = m_pieces.size() - 1;
            endOffsetInPiece = m_pieces.back().length;
        } else {
            return false;
        }
    }
    
    // Adjust end offset to be relative to original piece before any modifications
    if (endOffsetInPiece > 0 && endPieceIndex < m_pieces.size()) {
        // endOffsetInPiece is already correct from findPieceAt
    }
    
    // Handle removal within a single piece
    if (startPieceIndex == endPieceIndex) {
        Piece& piece = m_pieces[startPieceIndex];
        
        if (startOffsetInPiece == 0 && endOffsetInPiece == piece.length) {
            // Remove entire piece
            m_pieces.erase(m_pieces.begin() + startPieceIndex);
        } else if (startOffsetInPiece == 0) {
            // Remove from start of piece
            piece.bufferOffset += endOffsetInPiece;
            piece.length -= endOffsetInPiece;
        } else if (endOffsetInPiece == piece.length) {
            // Remove from end of piece
            piece.length = startOffsetInPiece;
        } else {
            // Remove from middle - split into two pieces
            Piece secondPiece(
                piece.bufferOffset + endOffsetInPiece,
                piece.length - endOffsetInPiece,
                piece.isFromOriginal
            );
            piece.length = startOffsetInPiece;
            m_pieces.insert(m_pieces.begin() + startPieceIndex + 1, secondPiece);
        }
    } else {
        // Removal spans multiple pieces
        
        // Handle start piece
        Piece& startPiece = m_pieces[startPieceIndex];
        if (startOffsetInPiece == 0) {
            // Will remove entire start piece and possibly more
        } else {
            // Trim start piece
            startPiece.length = startOffsetInPiece;
            startPieceIndex++;  // Start removing from next piece
        }
        
        // Handle end piece
        if (endPieceIndex < m_pieces.size()) {
            Piece& endPiece = m_pieces[endPieceIndex];
            if (endOffsetInPiece == endPiece.length) {
                // Will remove entire end piece
                endPieceIndex++;  // Remove through this piece
            } else {
                // Trim end piece from start
                endPiece.length -= endOffsetInPiece;
                endPiece.bufferOffset += endOffsetInPiece;
            }
        }
        
        // Remove pieces between start and end
        if (endPieceIndex > startPieceIndex) {
            m_pieces.erase(m_pieces.begin() + startPieceIndex, 
                          m_pieces.begin() + endPieceIndex);
        }
    }
    
    // Clean up empty pieces
    m_pieces.erase(
        std::remove_if(m_pieces.begin(), m_pieces.end(),
                      [](const Piece& p) { return p.length == 0; }),
        m_pieces.end()
    );
    
    m_totalLength -= removeLength;
    m_modified = true;
    rebuildLineIndex();
    
    return true;
}

std::string PieceTree::getText(size_t start, size_t length) const {
    std::string result;
    
    if (start >= m_totalLength || length == 0) {
        return result;
    }
    
    if (start + length > m_totalLength) {
        length = m_totalLength - start;
    }
    
    size_t remaining = length;
    size_t currentPos = 0;
    
    for (const auto& piece : m_pieces) {
        if (currentPos + piece.length <= start) {
            currentPos += piece.length;
            continue;
        }
        
        if (currentPos >= start + length) {
            break;
        }
        
        // Calculate overlap
        size_t pieceStart = currentPos;
        size_t pieceEnd = currentPos + piece.length;
        size_t rangeStart = start;
        size_t rangeEnd = start + length;
        
        size_t overlapStart = std::max(pieceStart, rangeStart);
        size_t overlapEnd = std::min(pieceEnd, rangeEnd);
        size_t overlapLength = overlapEnd - overlapStart;
        
        if (overlapLength > 0) {
            size_t offsetInPiece = overlapStart - pieceStart;
            const char* sourceData = piece.isFromOriginal ? m_originalBuffer : m_addBuffer.data();
            result.append(sourceData + piece.bufferOffset + offsetInPiece, overlapLength);
            remaining -= overlapLength;
        }
        
        currentPos += piece.length;
        
        if (remaining == 0) {
            break;
        }
    }
    
    return result;
}

std::string PieceTree::getCharAt(size_t pos) const {
    if (pos >= m_totalLength) {
        return "";
    }
    
    // Get one character - need to handle UTF-8 properly
    // For simplicity, get up to 4 bytes and determine actual length
    std::string text = getText(pos, 4);
    if (text.empty()) {
        return "";
    }
    
    int charLen = UtfCodec::getUtf8CharLength(text.data(), text.length(), 0);
    if (charLen <= 0) {
        charLen = 1;  // Fallback
    }
    
    return text.substr(0, static_cast<size_t>(charLen));
}

size_t PieceTree::getTotalLength() const {
    return m_totalLength;
}

size_t PieceTree::getLineCount() const {
    return m_lineOffsets.size();
}

TextPosition PieceTree::offsetToPosition(size_t byteOffset) const {
    TextPosition pos;
    
    if (byteOffset >= m_totalLength) {
        if (m_lineOffsets.empty()) {
            pos.line = 0;
            pos.column = 0;
        } else {
            pos.line = m_lineOffsets.size() - 1;
            pos.column = byteOffset - m_lineOffsets.back();
        }
        return pos;
    }
    
    // Binary search for the line
    auto it = std::upper_bound(m_lineOffsets.begin(), m_lineOffsets.end(), byteOffset);
    if (it == m_lineOffsets.begin()) {
        pos.line = 0;
        pos.column = byteOffset;
    } else {
        pos.line = static_cast<size_t>(it - m_lineOffsets.begin() - 1);
        pos.column = byteOffset - m_lineOffsets[pos.line];
    }
    
    return pos;
}

size_t PieceTree::positionToOffset(const TextPosition& position) const {
    if (position.line >= m_lineOffsets.size()) {
        return m_totalLength;
    }
    
    size_t lineStart = m_lineOffsets[position.line];
    size_t lineEnd = (position.line + 1 < m_lineOffsets.size()) 
                     ? m_lineOffsets[position.line + 1] 
                     : m_totalLength;
    
    size_t lineLength = lineEnd - lineStart;
    // Check if we're on the last line (which might not have a newline)
    if (position.line + 1 == m_lineOffsets.size()) {
        // Last line - column can extend to end of line
        size_t col = std::min(position.column, lineLength);
        return lineStart + col;
    } else {
        // Not last line - column should not include the newline character
        size_t col = std::min(position.column, lineLength > 0 ? lineLength - 1 : 0);
        return lineStart + col;
    }
}

size_t PieceTree::getLineStartOffset(size_t line) const {
    if (line >= m_lineOffsets.size()) {
        return INVALID_POSITION;
    }
    return m_lineOffsets[line];
}

size_t PieceTree::getLineLength(size_t line) const {
    if (line >= m_lineOffsets.size()) {
        return 0;
    }
    
    size_t lineStart = m_lineOffsets[line];
    size_t lineEnd = (line + 1 < m_lineOffsets.size()) 
                     ? m_lineOffsets[line + 1] 
                     : m_totalLength;
    
    size_t length = lineEnd - lineStart;
    // Exclude newline character if present
    if (length > 0 && line + 1 < m_lineOffsets.size()) {
        length--;
    }
    
    return length;
}

std::vector<std::string> PieceTree::getVisibleLines(size_t startLine, size_t endLine) const {
    std::vector<std::string> result;
    
    if (startLine >= m_lineOffsets.size()) {
        return result;
    }
    
    if (endLine >= m_lineOffsets.size()) {
        endLine = m_lineOffsets.size() - 1;
    }
    
    for (size_t line = startLine; line <= endLine; ++line) {
        size_t lineStart = m_lineOffsets[line];
        size_t lineLength = getLineLength(line);
        result.push_back(getText(lineStart, lineLength));
    }
    
    return result;
}

std::string PieceTree::getFullText() const {
    return getText(0, m_totalLength);
}

bool PieceTree::findPieceAt(size_t offset, size_t& outPieceIndex, size_t& outOffsetInPiece) const {
    if (offset > m_totalLength) {
        return false;
    }
    
    if (offset == m_totalLength) {
        // At the end of document
        if (m_pieces.empty()) {
            outPieceIndex = 0;
            outOffsetInPiece = 0;
            return false;
        }
        outPieceIndex = m_pieces.size() - 1;
        outOffsetInPiece = m_pieces.back().length;
        return true;
    }
    
    size_t currentPos = 0;
    for (size_t i = 0; i < m_pieces.size(); ++i) {
        const auto& piece = m_pieces[i];
        
        if (offset < currentPos + piece.length) {
            outPieceIndex = i;
            outOffsetInPiece = offset - currentPos;
            return true;
        }
        
        currentPos += piece.length;
    }
    
    // Should not reach here for valid offsets
    return false;
}

void PieceTree::splitPiece(size_t pieceIndex, size_t offsetInPiece) {
    if (pieceIndex >= m_pieces.size()) {
        return;
    }
    
    Piece& piece = m_pieces[pieceIndex];
    
    if (offsetInPiece == 0 || offsetInPiece >= piece.length) {
        return;  // Nothing to split
    }
    
    // Create new piece for the second half
    Piece secondPiece(
        piece.bufferOffset + offsetInPiece,
        piece.length - offsetInPiece,
        piece.isFromOriginal
    );
    
    // Truncate first piece
    piece.length = offsetInPiece;
    
    // Insert second piece after first
    m_pieces.insert(m_pieces.begin() + pieceIndex + 1, secondPiece);
}

void PieceTree::rebuildLineIndex() {
    m_lineOffsets.clear();
    
    if (m_totalLength == 0) {
        m_lineOffsets.push_back(0);  // Empty document has one empty line
        return;
    }
    
    // Iterate through all pieces and find line breaks
    size_t currentOffset = 0;
    m_lineOffsets.push_back(0);  // First line always starts at 0
    
    for (const auto& piece : m_pieces) {
        const char* data = piece.isFromOriginal ? m_originalBuffer : m_addBuffer.data();
        size_t pieceStart = piece.bufferOffset;
        size_t pieceEnd = piece.bufferOffset + piece.length;
        
        for (size_t i = pieceStart; i < pieceEnd; ++i) {
            if (data[i] == '\n') {
                // Found newline - next line starts after it
                size_t nextLineOffset = currentOffset + (i - pieceStart) + 1;
                // Add line start if it's within document bounds
                if (nextLineOffset <= m_totalLength) {
                    m_lineOffsets.push_back(nextLineOffset);
                }
            }
        }
        
        currentOffset += piece.length;
    }
}

} // namespace editor
