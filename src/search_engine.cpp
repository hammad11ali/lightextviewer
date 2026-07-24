#include "search_engine.h"
#include "utf_codec.h"
#include <algorithm>
#include <cwctype>

namespace editor {

SearchEngine::SearchResults SearchEngine::search(const PieceTree& document, 
                                                  const std::string& query,
                                                  bool caseSensitive) const {
    SearchResults results;
    
    if (query.empty()) {
        return results;
    }
    
    size_t docLength = document.getTotalLength();
    if (docLength == 0 || query.length() > docLength) {
        return results;
    }
    
    // For large files, we search in chunks to avoid memory issues
    constexpr size_t CHUNK_SIZE = 1024 * 1024;  // 1MB chunks
    
    size_t pos = 0;
    while (pos < docLength) {
        // Get chunk of text for searching
        size_t chunkLength = std::min(CHUNK_SIZE + query.length(), docLength - pos);
        std::string chunk = document.getText(pos, chunkLength);
        
        // Search within chunk
        size_t chunkPos = 0;
        while (chunkPos + query.length() <= chunk.length()) {
            size_t matchPos = 0;
            bool found = false;
            
            if (caseSensitive) {
                found = findCaseSensitive(chunk, query, matchPos);
            } else {
                found = findCaseInsensitive(chunk, query, matchPos);
            }
            
            if (!found || matchPos >= chunk.length()) {
                break;
            }
            
            // Verify match is within valid range (not overlapping chunk boundary incorrectly)
            if (matchPos + query.length() > chunk.length()) {
                break;
            }
            
            Match match;
            match.byteOffset = pos + matchPos;
            match.length = query.length();
            match.position = document.offsetToPosition(match.byteOffset);
            
            results.matches.push_back(match);
            results.totalCount++;
            
            // Cap results if too many
            if (results.totalCount >= MAX_SEARCH_RESULTS) {
                results.capped = true;
                results.isComplete = false;
                return results;
            }
            
            // Move past this match
            chunkPos = matchPos + 1;
            pos = pos + matchPos + 1;
        }
        
        // If we didn't find anything in this chunk, move forward
        if (chunkPos == 0) {
            pos += CHUNK_SIZE;
        }
    }
    
    results.isComplete = !results.capped;
    return results;
}

std::optional<SearchEngine::Match> SearchEngine::searchNext(const PieceTree& document,
                                                             const std::string& query,
                                                             size_t startPos,
                                                             bool caseSensitive) const {
    if (query.empty()) {
        return std::nullopt;
    }
    
    size_t docLength = document.getTotalLength();
    if (startPos >= docLength || query.length() > docLength) {
        return std::nullopt;
    }
    
    // Search from startPos to end
    size_t searchLength = docLength - startPos;
    std::string text = document.getText(startPos, searchLength);
    
    size_t matchPos = 0;
    bool found = false;
    
    if (caseSensitive) {
        found = findCaseSensitive(text, query, matchPos);
    } else {
        found = findCaseInsensitive(text, query, matchPos);
    }
    
    if (!found) {
        return std::nullopt;
    }
    
    Match match;
    match.byteOffset = startPos + matchPos;
    match.length = query.length();
    match.position = document.offsetToPosition(match.byteOffset);
    
    return match;
}

std::optional<SearchEngine::Match> SearchEngine::searchPrevious(const PieceTree& document,
                                                                 const std::string& query,
                                                                 size_t endPos,
                                                                 bool caseSensitive) const {
    if (query.empty() || endPos == 0) {
        return std::nullopt;
    }
    
    size_t docLength = document.getTotalLength();
    if (query.length() > docLength) {
        return std::nullopt;
    }
    
    // Search from beginning to endPos
    std::string text = document.getText(0, endPos);
    
    // Find last occurrence
    size_t lastMatch = std::string::npos;
    size_t searchPos = 0;
    
    while (searchPos + query.length() <= text.length()) {
        size_t matchPos = 0;
        bool found = false;
        
        if (caseSensitive) {
            found = findCaseSensitive(text.substr(searchPos), query, matchPos);
        } else {
            found = findCaseInsensitive(text.substr(searchPos), query, matchPos);
        }
        
        if (!found) {
            break;
        }
        
        lastMatch = searchPos + matchPos;
        searchPos = searchPos + matchPos + 1;
    }
    
    if (lastMatch == std::string::npos) {
        return std::nullopt;
    }
    
    Match match;
    match.byteOffset = lastMatch;
    match.length = query.length();
    match.position = document.offsetToPosition(match.byteOffset);
    
    return match;
}

bool SearchEngine::replace(PieceTree& document, const Match& match, 
                           const std::string& replacement) const {
    if (match.byteOffset >= document.getTotalLength()) {
        return false;
    }
    
    return document.remove(match.byteOffset, match.byteOffset + match.length) &&
           document.insert(match.byteOffset, replacement);
}

size_t SearchEngine::replaceAll(PieceTree& document, const std::string& query,
                                const std::string& replacement, bool caseSensitive) const {
    if (query.empty()) {
        return 0;
    }
    
    // Find all matches first
    SearchResults results = search(document, query, caseSensitive);
    if (results.matches.empty()) {
        return 0;
    }
    
    // Replace from end to start to preserve offsets
    size_t replaceCount = 0;
    for (auto it = results.matches.rbegin(); it != results.matches.rend(); ++it) {
        if (replace(document, *it, replacement)) {
            replaceCount++;
        }
    }
    
    return replaceCount;
}

bool SearchEngine::findCaseSensitive(const std::string& text, const std::string& pattern,
                                     size_t& pos) const {
    pos = text.find(pattern);
    return pos != std::string::npos;
}

bool SearchEngine::findCaseInsensitive(const std::string& text, const std::string& pattern,
                                       size_t& pos) const {
    if (pattern.empty() || text.empty() || pattern.length() > text.length()) {
        return false;
    }
    
    std::string textLower = toLower(text);
    std::string patternLower = toLower(pattern);
    
    pos = textLower.find(patternLower);
    return pos != std::string::npos;
}

std::string SearchEngine::toLower(const std::string& str) const {
    // Convert UTF-8 string to lowercase
    // This is a simplified implementation that handles ASCII well
    // For full Unicode case folding, a more complex implementation would be needed
    
    std::wstring wide;
    auto result = UtfCodec::utf8ToUtf16(str);
    if (!result.success) {
        // Fallback: just return original
        return str;
    }
    wide = result.utf16;
    
    // Convert to lowercase using wide character functions
    for (wchar_t& c : wide) {
        c = std::towlower(c);
    }
    
    // Convert back to UTF-8
    auto utf8Result = UtfCodec::utf16ToUtf8(wide);
    if (!utf8Result.success) {
        return str;
    }
    
    return utf8Result.utf8;
}

} // namespace editor
