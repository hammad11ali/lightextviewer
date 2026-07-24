#include "renderer.h"
#include <dwrite.h>
#include <d2d1.h>
#include <vector>
#include <string>

#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")

namespace editor {

// Helper function to convert UTF-8 to UTF-16
static std::wstring utf8ToUtf16(const std::string& utf8) {
    if (utf8.empty()) return L"";
    
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    
    std::wstring result(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], len);
    return result;
}

bool Renderer::initialize(void* hwnd) {
#ifdef _WIN32
    if (!hwnd) return false;
    
    HRESULT hr;
    
    // Create Direct2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, 
                           __uuidof(ID2D1Factory), 
                           reinterpret_cast<void**>(&m_d2dFactory));
    if (FAILED(hr)) return false;
    
    // Create DirectWrite factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, 
                             __uuidof(IDWriteFactory), 
                             reinterpret_cast<IUnknown**>(&m_dwriteFactory));
    if (FAILED(hr)) {
        m_d2dFactory->Release();
        m_d2dFactory = nullptr;
        return false;
    }
    
    // Create text format (Consolas, 14pt)
    hr = m_dwriteFactory->CreateTextFormat(
        L"Consolas", 
        nullptr, 
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, 
        DWRITE_FONT_STRETCH_NORMAL, 
        14.0f, 
        L"en-us", 
        &m_textFormat);
    if (FAILED(hr)) {
        cleanup();
        return false;
    }
    
    // Get client area size
    RECT rc;
    GetClientRect(static_cast<HWND>(hwnd), &rc);
    m_windowWidth = rc.right - rc.left;
    m_windowHeight = rc.bottom - rc.top;
    
    // Create render target
    D2D1_SIZE_U size = D2D1::SizeU(m_windowWidth, m_windowHeight);
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f, 96.0f,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT);
    
    hr = m_d2dFactory->CreateHwndRenderTarget(
        props,
        D2D1::HwndRenderTargetProperties(static_cast<HWND>(hwnd), size),
        &m_renderTarget);
    if (FAILED(hr)) {
        cleanup();
        return false;
    }
    
    // Create brushes
    // Text brush (black)
    hr = m_renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush);
    if (FAILED(hr)) {
        cleanup();
        return false;
    }
    
    // Selection brush (light blue with transparency)
    hr = m_renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightBlue, 0.3f), &m_selectionBrush);
    if (FAILED(hr)) {
        cleanup();
        return false;
    }
    
    // Search highlight brush (yellow with transparency)
    hr = m_renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Yellow, 0.5f), &m_highlightBrush);
    if (FAILED(hr)) {
        cleanup();
        return false;
    }
    
    // Get font metrics to calculate line height
    DWRITE_FONT_METRICS fontMetrics;
    m_textFormat->GetFontMetrics(&fontMetrics);
    float pixelsPerDip = m_renderTarget->GetDpiY() / 72.0f;
    m_lineHeight = static_cast<int>((fontMetrics.ascent + fontMetrics.descent) * 
                                     fontMetrics.designUnitsPerEm * pixelsPerDip / 1000.0f) + 2;
    if (m_lineHeight < 16) m_lineHeight = 16;
    
    return true;
#else
    (void)hwnd;
    return false;
#endif
}

void Renderer::cleanup() {
#ifdef _WIN32
    // Release DirectWrite and Direct2D resources
    if (m_selectionBrush) m_selectionBrush->Release();
    if (m_highlightBrush) m_highlightBrush->Release();
    if (m_textBrush) m_textBrush->Release();
    if (m_renderTarget) m_renderTarget->Release();
    if (m_d2dFactory) m_d2dFactory->Release();
    if (m_textFormat) m_textFormat->Release();
    if (m_dwriteFactory) m_dwriteFactory->Release();
    
    m_selectionBrush = nullptr;
    m_highlightBrush = nullptr;
    m_textBrush = nullptr;
    m_renderTarget = nullptr;
    m_d2dFactory = nullptr;
    m_textFormat = nullptr;
    m_dwriteFactory = nullptr;
#endif
}

void Renderer::render(const std::vector<std::string>& lines,
                      size_t firstVisibleLine,
                      size_t caretLine, size_t caretColumn,
                      size_t selectionStart, size_t selectionEnd) {
#ifdef _WIN32
    if (!m_renderTarget || !m_textBrush || !m_textFormat) return;
    
    m_renderTarget->BeginDraw();
    
    // Clear background to white
    m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
    
    // Calculate vertical offset based on scroll position
    float yOffset = static_cast<float>(-(static_cast<int>(firstVisibleLine) * m_lineHeight));
    float xOffset = static_cast<float>(-m_horizontalScrollOffset);
    
    // Character width approximation (Consolas is monospace)
    const float charWidth = 8.5f;
    
    // Draw each visible line
    for (size_t i = 0; i < lines.size(); ++i) {
        size_t lineNum = firstVisibleLine + i;
        float lineY = yOffset + static_cast<float>(i * m_lineHeight);
        
        // Skip if line is outside viewport
        if (lineY + m_lineHeight < 0 || lineY > static_cast<float>(m_windowHeight)) continue;
        
        const std::string& lineText = lines[i];
        
        // Convert line to UTF-16 for DirectWrite
        std::wstring wideLine = utf8ToUtf16(lineText);
        
        // Calculate line length in pixels for horizontal scrolling
        float lineWidth = static_cast<float>(wideLine.length()) * charWidth;
        
        // Check if this line has selection or caret
        bool hasSelectionOnLine = (selectionStart <= lineNum && lineNum <= selectionEnd && selectionStart != selectionEnd);
        bool isCaretLine = (lineNum == caretLine);
        
        // Draw selection background if applicable
        if (hasSelectionOnLine) {
            size_t selStartCol = (lineNum == selectionStart) ? 0 : 0;
            size_t selEndCol = (lineNum == selectionEnd) ? lineText.length() : lineText.length();
            
            float selX = xOffset + static_cast<float>(selStartCol) * charWidth;
            float selWidth = static_cast<float>(selEndCol - selStartCol) * charWidth;
            
            if (selWidth > 0 && selX < static_cast<float>(m_windowWidth)) {
                D2D1_RECT_F selRect = D2D1::RectF(
                    std::max(0.0f, selX),
                    lineY,
                    std::min(selX + selWidth, static_cast<float>(m_windowWidth)),
                    lineY + static_cast<float>(m_lineHeight));
                m_renderTarget->FillRectangle(&selRect, m_selectionBrush);
            }
        }
        
        // Draw text
        if (!wideLine.empty()) {
            // Create text layout for this line
            IDWriteTextLayout* textLayout = nullptr;
            HRESULT hr = m_dwriteFactory->CreateTextLayout(
                wideLine.c_str(),
                static_cast<UINT32>(wideLine.length()),
                m_textFormat,
                10000.0f,  // Max width (large enough for long lines)
                static_cast<float>(m_lineHeight),
                &textLayout);
            
            if (SUCCEEDED(hr) && textLayout) {
                // Apply horizontal scroll offset
                DWRITE_TEXT_METRICS textMetrics;
                textLayout->GetMetrics(&textMetrics);
                
                // Draw the text at the calculated position
                m_renderTarget->DrawTextLayout(
                    D2D1::Point2F(std::max(0.0f, xOffset), lineY),
                    textLayout,
                    m_textBrush);
                
                textLayout->Release();
            }
        }
        
        // Draw caret if this is the caret line
        if (isCaretLine && caretColumn <= lineText.length()) {
            float caretX = xOffset + static_cast<float>(caretColumn) * charWidth;
            
            // Only draw caret if it's within visible area
            if (caretX >= 0 && caretX < static_cast<float>(m_windowWidth)) {
                D2D1_RECT_F caretRect = D2D1::RectF(
                    caretX,
                    lineY + 2.0f,
                    caretX + 1.5f,
                    lineY + static_cast<float>(m_lineHeight) - 2.0f);
                m_renderTarget->FillRectangle(&caretRect, m_textBrush);
            }
        }
    }
    
    HRESULT hr = m_renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        // Handle device lost scenario - would need to recreate resources
        m_renderTarget = nullptr;
    }
#else
    (void)lines;
    (void)firstVisibleLine;
    (void)caretLine;
    (void)caretColumn;
    (void)selectionStart;
    (void)selectionEnd;
#endif
}

void Renderer::onResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    
#ifdef _WIN32
    // Update render target size
    if (m_renderTarget) {
        m_renderTarget->Resize(D2D1::SizeU(width, height));
    }
#endif
}

size_t Renderer::getVisibleLineCount() const {
    if (m_lineHeight <= 0) {
        return 0;
    }
    return static_cast<size_t>(m_windowHeight / m_lineHeight) + 1;
}

size_t Renderer::getVisibleColumnCount() const {
    // Approximate: assuming ~8 pixels per character for Consolas at 14pt
    return static_cast<size_t>(m_windowWidth / 8);
}

void Renderer::scrollToLine(size_t line) {
    m_firstVisibleLine = line;
}

void Renderer::scrollHorizontalToColumn(size_t column) {
    // Approximate: 8 pixels per character
    m_horizontalScrollOffset = static_cast<int>(column * 8);
}

} // namespace editor
