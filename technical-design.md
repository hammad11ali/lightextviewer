# Final Technical Design Plan

## Lightweight Large-File Text Editor for Windows

**Version:** 2.0  
**Date:** 2026-07-24  
**Status:** Final Recommended Design

---

## 1. Purpose

This document defines the final technical design for a **lightweight Windows-only plain-text editor** capable of:

- Opening very large text files
- Handling extremely long single-line files
- Displaying Urdu text correctly
- Handling UTF-8 correctly
- Supporting copy/paste
- Supporting undo/redo
- Supporting search
- Supporting search-result highlighting
- Supporting match count
- Supporting case-sensitive search toggle
- Supporting find and replace
- Supporting save

The editor is intentionally minimal and optimized for performance, low memory usage, and stability.

---

## 2. Scope

### 2.1 In Scope

- Windows-only desktop application
- Plain-text editing
- UTF-8 file handling
- Urdu text display
- Large-file support
- Large single-line support
- Copy and paste
- Undo and redo
- Search
- Search highlighting
- Match count
- Case-sensitive search toggle
- Find and replace
- Replace Next
- Replace All
- Save
- Minimal UI
- Low memory usage

### 2.2 Out of Scope

- Syntax highlighting
- Themes
- Plugins
- Extensions
- Macros
- Scripting
- Multiple tabs
- Multiple windows
- File explorer
- Project/workspace features
- Regular expression search
- Find in files
- Autosave
- Version control integration
- Spell checking
- Cloud sync
- Printing
- Export/conversion tools
- Telemetry
- Network access
- Non-Windows platforms

---

## 3. Encoding Strategy

### 3.1 Question: Should the editor use UTF-16 everywhere?

### Short Answer

Technically yes, but it is **not recommended** as the canonical internal encoding for this project.

The recommended design is:

> **UTF-8 for internal storage and editing.**  
> **UTF-16 for rendering, clipboard, and Windows interop.**

---

### 3.2 Why Not UTF-16 Everywhere?

| Concern | UTF-16 Everywhere | UTF-8 Internal + UTF-16 Transient |
|---|---|---|
| ASCII-heavy large files | Uses 2 bytes per ASCII character | Uses 1 byte per ASCII character |
| Memory usage for 1 GiB ASCII file | ~2 GiB if fully converted | ~1 GiB or less with memory mapping |
| DirectWrite compatibility | Direct | Requires decoding visible ranges |
| Windows clipboard | Direct | Requires conversion |
| UTF-8 file persistence | Requires conversion | Native |
| Memory-mapped UTF-8 file | Requires conversion or dual storage | Can use offsets directly |
| Invalid UTF-8 handling | Often lossy unless specially preserved | Can preserve or replace more easily |
| Large-file friendliness | Worse | Better |
| Implementation simplicity | Simpler at first | Slightly more complex, but more scalable |

---

### 3.3 Final Encoding Decision

### Canonical Storage Encoding

The editor shall use **UTF-8** as the canonical storage encoding.

This applies to:

- Original file buffer
- Add buffer
- Piece table byte offsets
- Saved file output
- Internal edit operations

### Presentation / Interop Encoding

The editor shall use **UTF-16** as a transient presentation encoding for:

- DirectWrite rendering
- Windows clipboard operations
- Some Windows text APIs
- Case-insensitive Unicode operations where practical

---

### 3.4 Encoding Rules

1. Files are opened as UTF-8.
2. ASCII is supported as a subset of UTF-8.
3. UTF-8 with or without BOM shall be supported.
4. If a BOM is present, it shall be detected.
5. On save:
   - If the original file had a UTF-8 BOM, preserve it.
   - If the original file had no BOM, do not add one.
6. Invalid UTF-8 bytes shall be handled gracefully.
7. Invalid bytes may be displayed as `U+FFFD`.
8. Saving may persist replacement characters for invalid bytes unless raw-byte preservation is explicitly implemented later.
9. UTF-16 shall not be used as the primary on-disk or in-memory document storage format for the MVP.
10. UTF-16 surrogate pairs must still be handled correctly when converting, rendering, selecting, and searching.

---

## 4. Functional Summary

The editor shall support:

- Open file
- Save file
- Plain-text display
- UTF-8 handling
- Urdu text display
- Caret and selection
- Copy
- Paste
- Basic text insertion/deletion
- Undo
- Redo
- Search
- Search highlighting
- Match count
- Case-sensitive search toggle
- Replace Next
- Replace All
- Replace count
- Scrollable viewport
- Horizontal virtualization for long lines
- Minimal error handling

---

## 5. Technology Stack

| Layer | Technology | Rationale |
|---|---|---|
| Language | C++20 | Performance, memory control, direct Windows API access |
| UI Framework | Raw Win32 | Minimal dependencies, low overhead |
| Text Rendering | DirectWrite | Urdu/Arabic shaping, RTL support, font fallback |
| File I/O | Win32 + memory-mapped files | Efficient large-file access |
| Build System | CMake + MSVC | Standard Windows C++ toolchain |
| Packaging | Portable `.exe` and optional MSI | Lightweight distribution |

---

## 6. High-Level Architecture

```text
┌─────────────────────────────────────────────────────────┐
│                    Main Window (Win32)                  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │              Text Viewport (DirectWrite)          │  │
│  │  ┌──────────┐  ┌──────────────────────────────┐   │  │
│  │  │ Optional │  │   Virtualized Text Surface    │   │  │
│  │  │ Gutter   │  │   Vertical + Horizontal       │   │  │
│  │  └──────────┘  └──────────────────────────────┘   │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │           Search / Replace Bar (Overlay)          │  │
│  │  Find: [____] Replace: [____] [Aa] [<] [>]        │  │
│  │  [Replace] [Replace All] [count] [×]              │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │                    Status Bar                     │  │
│  │  [file path] [Ln x, Col y] [UTF-8] [modified]     │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    Core Engine Layer                    │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ PieceTree    │  │ SearchEngine │  │ UndoManager  │  │
│  │ Text Buffer  │  │ ReplaceEngine│  │ Macro Undo   │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ MappedView   │  │ Utf8Utf16    │  │ Clipboard    │  │
│  │ Manager      │  │ Codec        │  │ Handler      │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ LineIndex    │  │ LayoutCache  │  │ DocumentEpoch│  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────┘