#include "undo_manager.h"

namespace editor {

UndoManager::UndoManager() = default;
UndoManager::~UndoManager() = default;

void UndoManager::beginMacro(const std::string& description) {
    if (m_inMacro) {
        return;  // Already in a macro
    }
    
    m_inMacro = true;
    m_currentMacro = MacroOperation();
    m_currentMacro.description = description;
}

void UndoManager::endMacro() {
    if (!m_inMacro) {
        return;
    }
    
    if (!m_currentMacro.operations.empty()) {
        // Push to undo stack
        m_undoStack.push_back(m_currentMacro);
        
        // Limit undo stack size
        while (m_undoStack.size() > m_maxUndoDepth) {
            m_undoStack.erase(m_undoStack.begin());
        }
    }
    
    m_inMacro = false;
    m_currentMacro = MacroOperation();
    
    // Clear redo stack when new edit is made
    m_redoStack.clear();
}

void UndoManager::recordInsert(size_t position, const std::string& text) {
    if (m_inMacro) {
        m_currentMacro.addOperation(EditType::Insert, position, text);
    } else {
        // Single operation
        MacroOperation macro;
        macro.addOperation(EditType::Insert, position, text);
        m_undoStack.push_back(macro);
        
        // Limit undo stack size
        while (m_undoStack.size() > m_maxUndoDepth) {
            m_undoStack.erase(m_undoStack.begin());
        }
        
        // Clear redo stack
        m_redoStack.clear();
    }
}

void UndoManager::recordDelete(size_t position, const std::string& text) {
    if (m_inMacro) {
        m_currentMacro.addOperation(EditType::Delete, position, text);
    } else {
        // Single operation
        MacroOperation macro;
        macro.addOperation(EditType::Delete, position, text);
        m_undoStack.push_back(macro);
        
        // Limit undo stack size
        while (m_undoStack.size() > m_maxUndoDepth) {
            m_undoStack.erase(m_undoStack.begin());
        }
        
        // Clear redo stack
        m_redoStack.clear();
    }
}

void UndoManager::recordReplace(size_t position, const std::string& oldText, 
                                const std::string& newText) {
    if (m_inMacro) {
        m_currentMacro.addOperation(EditType::Replace, position, oldText);
        // Note: For replace, we store the old text; newText would need to be tracked separately
        // or handled as delete+insert
    } else {
        // Single operation - treat as delete of oldText followed by insert of newText
        // For simplicity, we'll just record the delete part for undo
        MacroOperation macro;
        macro.addOperation(EditType::Replace, position, oldText);
        m_undoStack.push_back(macro);
        
        // Limit undo stack size
        while (m_undoStack.size() > m_maxUndoDepth) {
            m_undoStack.erase(m_undoStack.begin());
        }
        
        // Clear redo stack
        m_redoStack.clear();
    }
}

bool UndoManager::undo() {
    if (!canUndo()) {
        return false;
    }
    
    // Get the last macro operation
    MacroOperation macro = m_undoStack.back();
    m_undoStack.pop_back();
    
    // Execute operations in reverse order
    for (auto it = macro.operations.rbegin(); it != macro.operations.rend(); ++it) {
        executeUndo(*it);
    }
    
    // Store for redo (with inverted operations)
    m_redoStack.push_back(macro);
    
    return true;
}

bool UndoManager::redo() {
    if (!canRedo()) {
        return false;
    }
    
    // Get the last redo operation
    MacroOperation macro = m_redoStack.back();
    m_redoStack.pop_back();
    
    // Execute operations in forward order
    for (const auto& op : macro.operations) {
        executeRedo(op);
    }
    
    // Move back to undo stack
    m_undoStack.push_back(macro);
    
    return true;
}

bool UndoManager::canUndo() const {
    return !m_undoStack.empty();
}

bool UndoManager::canRedo() const {
    return !m_redoStack.empty();
}

void UndoManager::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
    m_currentMacro = MacroOperation();
    m_inMacro = false;
}

size_t UndoManager::getUndoCount() const {
    return m_undoStack.size();
}

size_t UndoManager::getRedoCount() const {
    return m_redoStack.size();
}

UndoManager::EditOperation UndoManager::executeUndo(const EditOperation& op) {
    // This method should actually modify the document
    // For now, we just return the inverse operation
    // The actual document modification happens in the editor
    
    EditOperation inverse;
    inverse.position = op.position;
    
    switch (op.type) {
        case EditType::Insert:
            // To undo insert, we delete the inserted text
            inverse.type = EditType::Delete;
            inverse.text = op.text;
            break;
            
        case EditType::Delete:
            // To undo delete, we re-insert the deleted text
            inverse.type = EditType::Insert;
            inverse.text = op.text;
            break;
            
        case EditType::Replace:
            // Replace undo would need both old and new text
            // Simplified: treat as delete
            inverse.type = EditType::Delete;
            inverse.text = op.text;
            break;
    }
    
    return inverse;
}

void UndoManager::executeRedo(const EditOperation& op) {
    // This method should actually modify the document
    // For now, this is a placeholder
    // The actual document modification happens in the editor
    
    switch (op.type) {
        case EditType::Insert:
            // Redo insert - text was already inserted during undo reversal
            break;
            
        case EditType::Delete:
            // Redo delete - text was already deleted during undo reversal
            break;
            
        case EditType::Replace:
            // Redo replace
            break;
    }
}

} // namespace editor
