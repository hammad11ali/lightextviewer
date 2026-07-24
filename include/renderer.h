#pragma once

#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <dwrite.h>
#include <d2d1.h>
#endif

namespace editor {

/**
 * Renderer for displaying text using DirectWrite
 * Handles text layout, font rendering, and scrolling
 */
class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;
    
    /**
     * Initialize the renderer
     * @param hwnd Window handle for rendering
     * @return true if successful
     */
    bool initialize(void* hwnd);
    
    /**
     * Clean up renderer resources
     */
    void cleanup();
    
    /**
     * Render text to the window
     * @param text UTF-8 text to render
     * @param lines Vector of line strings
     * @param firstVisibleLine First visible line number
     * @param caretLine Current caret line
     * @param caretColumn Current caret column
     * @param selectionStart Selection start position (line)
     * @param selectionEnd Selection end position (line)
     */
    void render(const std::vector<std::string>& lines, 
                size_t firstVisibleLine,
                size_t caretLine, size_t caretColumn,
                size_t selectionStart, size_t selectionEnd);
    
    /**
     * Handle window resize
     * @param width New width
     * @param height New height
     */
    void onResize(int width, int height);
    
    /**
     * Get the number of visible lines in current viewport
     * @return Number of visible lines
     */
    size_t getVisibleLineCount() const;
    
    /**
     * Get the number of visible columns in current viewport
     * @return Number of visible columns (approximate)
     */
    size_t getVisibleColumnCount() const;
    
    /**
     * Scroll to a specific line
     * @param line Line number to scroll to
     */
    void scrollToLine(size_t line);
    
    /**
     * Scroll horizontally to a specific column
     * @param column Column to scroll to
     */
    void scrollHorizontalToColumn(size_t column);
    
    /**
     * Get current vertical scroll position (first visible line)
     * @return First visible line number
     */
    size_t getScrollPosition() const { return m_firstVisibleLine; }
    
    /**
     * Get current horizontal scroll offset
     * @return Horizontal scroll offset in pixels
     */
    int getHorizontalScrollOffset() const { return m_horizontalScrollOffset; }
    
private:
    size_t m_firstVisibleLine = 0;
    int m_horizontalScrollOffset = 0;
    int m_windowWidth = 800;
    int m_windowHeight = 600;
    int m_lineHeight = 16;  // Approximate line height in pixels
    
#ifdef _WIN32
    // DirectWrite and Direct2D resources would be stored here
    IDWriteFactory* m_dwriteFactory = nullptr;
    IDWriteTextFormat* m_textFormat = nullptr;
    ID2D1Factory* m_d2dFactory = nullptr;
    ID2D1HwndRenderTarget* m_renderTarget = nullptr;
    ID2D1SolidColorBrush* m_textBrush = nullptr;
    ID2D1SolidColorBrush* m_highlightBrush = nullptr;
    ID2D1SolidColorBrush* m_selectionBrush = nullptr;
#endif
};

} // namespace editor
