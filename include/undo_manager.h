#pragma once

#include <string>
#include <vector>
#include <memory>
#include "types.h"

namespace editor {

/**
 * Undo/Redo manager for tracking document edits
 * Supports macro operations for grouping multiple edits
 */
class UndoManager {
public:
    /**
     * Types of edit operations
     */
    enum class EditType {
        Insert,
        Delete,
        Replace
    };
    
    /**
     * Represents a single edit operation
     */
    struct EditOperation {
        EditType type = EditType::Insert;
        size_t position = 0;       // Byte offset where edit occurred
        std::string text;           // Text that was inserted or deleted
        bool isUndo = false;        // True if this represents undo state
        
        EditOperation() = default;
        EditOperation(EditType t, size_t pos, const std::string& txt)
            : type(t), position(pos), text(txt) {}
    };
    
    /**
     * Macro operation - groups multiple edits into one undoable action
     */
    struct MacroOperation {
        std::vector<EditOperation> operations;
        std::string description;
        
        void addOperation(EditType type, size_t pos, const std::string& text) {
            operations.emplace_back(type, pos, text);
        }
    };
    
    UndoManager();
    ~UndoManager();
    
    /**
     * Begin a macro operation (groups subsequent edits)
     * @param description Optional description of the operation
     */
    void beginMacro(const std::string& description = "");
    
    /**
     * End a macro operation
     */
    void endMacro();
    
    /**
     * Record an insert operation
     * @param position Byte offset where text was inserted
     * @param text The inserted text
     */
    void recordInsert(size_t position, const std::string& text);
    
    /**
     * Record a delete operation
     * @param position Byte offset where text was deleted
     * @param text The deleted text
     */
    void recordDelete(size_t position, const std::string& text);
    
    /**
     * Record a replace operation
     * @param position Byte offset where replacement occurred
     * @param oldText The text that was replaced
     * @param newText The replacement text
     */
    void recordReplace(size_t position, const std::string& oldText, const std::string& newText);
    
    /**
     * Undo the last operation
     * @return true if undo was performed, false if nothing to undo
     */
    bool undo();
    
    /**
     * Redo the last undone operation
     * @return true if redo was performed, false if nothing to redo
     */
    bool redo();
    
    /**
     * Check if undo is available
     * @return true if there are operations to undo
     */
    bool canUndo() const;
    
    /**
     * Check if redo is available
     * @return true if there are operations to redo
     */
    bool canRedo() const;
    
    /**
     * Clear all undo/redo history
     */
    void clear();
    
    /**
     * Get the number of undo operations available
     * @return Number of undoable operations
     */
    size_t getUndoCount() const;
    
    /**
     * Get the number of redo operations available
     * @return Number of redoable operations
     */
    size_t getRedoCount() const;
    
private:
    /**
     * Execute an edit operation in reverse (for undo)
     * @param op The operation to reverse
     * @return The inverse operation (for redo)
     */
    EditOperation executeUndo(const EditOperation& op);
    
    /**
     * Execute an edit operation forward (for redo)
     * @param op The operation to execute
     */
    void executeRedo(const EditOperation& op);
    
    std::vector<MacroOperation> m_undoStack;      // Stack of undoable operations
    std::vector<MacroOperation> m_redoStack;      // Stack of redoable operations
    
    MacroOperation m_currentMacro;                // Current macro being built
    bool m_inMacro = false;                       // Whether we're building a macro
    
    size_t m_maxUndoDepth = 1000;                 // Maximum undo history depth
};

} // namespace editor
