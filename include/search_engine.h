#pragma once

#include <string>
#include <vector>
#include <functional>
#include "types.h"
#include "piece_tree.h"

namespace editor {

/**
 * Search engine for finding text in documents
 * Supports case-sensitive/insensitive search
 * Works efficiently with large files through incremental search
 */
class SearchEngine {
public:
    /**
     * Search result with position information
     */
    struct Match {
        size_t byteOffset = 0;      // Byte offset in document
        size_t length = 0;           // Length of match in bytes
        TextPosition position;       // Line/column position
        
        bool operator==(const Match& other) const = default;
    };
    
    /**
     * Search results container
     */
    struct SearchResults {
        std::vector<Match> matches;
        size_t totalCount = 0;       // Total matches (may be capped)
        bool isComplete = false;     // True if all matches found
        bool capped = false;         // True if count was capped
    };
    
    SearchEngine() = default;
    ~SearchEngine() = default;
    
    /**
     * Search for text in the document
     * @param document The piece tree to search
     * @param query The search query (UTF-8)
     * @param caseSensitive Whether search is case-sensitive
     * @return SearchResults with all matches
     */
    SearchResults search(const PieceTree& document, const std::string& query, 
                        bool caseSensitive = true) const;
    
    /**
     * Search for next match after a given position
     * @param document The piece tree to search
     * @param query The search query (UTF-8)
     * @param startPos Position to start searching from
     * @param caseSensitive Whether search is case-sensitive
     * @return Match if found, empty optional otherwise
     */
    std::optional<Match> searchNext(const PieceTree& document, const std::string& query,
                                   size_t startPos = 0, bool caseSensitive = true) const;
    
    /**
     * Search for previous match before a given position
     * @param document The piece tree to search
     * @param query The search query (UTF-8)
     * @param endPos Position to search backwards to
     * @param caseSensitive Whether search is case-sensitive
     * @return Match if found, empty optional otherwise
     */
    std::optional<Match> searchPrevious(const PieceTree& document, const std::string& query,
                                       size_t endPos = 0, bool caseSensitive = true) const;
    
    /**
     * Replace a match in the document
     * @param document The piece tree to modify
     * @param match The match to replace
     * @param replacement The replacement text (UTF-8)
     * @return true if successful
     */
    bool replace(PieceTree& document, const Match& match, const std::string& replacement) const;
    
    /**
     * Replace all matches in the document
     * @param document The piece tree to modify
     * @param query The search query (UTF-8)
     * @param replacement The replacement text (UTF-8)
     * @param caseSensitive Whether search is case-sensitive
     * @return Number of replacements made
     */
    size_t replaceAll(PieceTree& document, const std::string& query,
                     const std::string& replacement, bool caseSensitive = true) const;
    
private:
    /**
     * Perform case-insensitive comparison
     * @param text Text to search in (UTF-8)
     * @param pattern Pattern to search for (UTF-8)
     * @param pos Output: position where match was found
     * @return true if match found
     */
    bool findCaseInsensitive(const std::string& text, const std::string& pattern,
                            size_t& pos) const;
    
    /**
     * Perform case-sensitive comparison
     * @param text Text to search in (UTF-8)
     * @param pattern Pattern to search for (UTF-8)
     * @param pos Output: position where match was found
     * @return true if match found
     */
    bool findCaseSensitive(const std::string& text, const std::string& pattern,
                          size_t& pos) const;
    
    /**
     * Convert string to lowercase for case-insensitive comparison
     * @param str Input string (UTF-8)
     * @return Lowercase string
     */
    std::string toLower(const std::string& str) const;
};

} // namespace editor
