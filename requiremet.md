# Requirement Document

**Lightweight Large-File Text Editor for Windows**

Version: 1.1  
Date: 2026-07-24  
Status: Updated Draft

---

## 1. Purpose

This document defines the requirements for a minimal, lightweight Windows plain-text editor designed to:

- Open and edit large text files
- Handle extremely long single-line files
- Support UTF-8
- Display Urdu text
- Provide only essential functionality:
  - Copy
  - Paste
  - Undo
  - Redo
  - Search
  - Search-result highlighting
  - Match count
  - Case-sensitive search toggle
  - Find and replace
  - Save

The product is intentionally lightweight and excludes advanced editor features.

---

## 2. Scope

### 2.1 In Scope

The product shall provide:

- Plain-text editing
- UTF-8 file handling
- Urdu text display
- Large-file support
- Long single-line file support
- Copy and paste
- Undo and redo
- Search
- Search-result highlighting
- Match count display
- Case-sensitive search toggle
- Find and replace
- Save functionality
- Minimal UI
- Windows-only support

### 2.2 Out of Scope

The following are explicitly excluded unless added later:

- Syntax highlighting
- Code folding
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
- Non-Windows platform support

---

## 3. Target Platform

### 3.1 Required Platform

The editor shall target Windows only for now.

### 3.2 Supported Windows Versions

**Minimum target:**

- Windows 10 64-bit
- Windows 11 64-bit

**Optional future support may include:**

- Windows on ARM64

---

## 4. Assumptions

- The editor is intended for plain-text files only.
- The primary encoding is UTF-8.
- "Highlight" means search-result highlighting, not syntax highlighting.
- Urdu text display requires proper Unicode rendering and right-to-left support.
- Basic caret movement, scrolling, and selection are necessary supporting features.
- Basic text insertion and deletion are part of normal editing.
- Save is now a required feature.
- Match count and case-sensitive search toggle are now required features.

---

## 5. Primary Use Cases

### 5.1 Large File Viewing and Editing

The user shall be able to:

- Open a large text file.
- Scroll without freezing.
- Search for text.
- See highlighted matches.
- See the number of matches.
- Copy and paste text.
- Undo and redo edits.
- Save the file.

### 5.2 Urdu Text Handling

The user shall be able to:

- Open a UTF-8 file containing Urdu text.
- See Urdu characters displayed correctly.
- Search for Urdu text.
- See Urdu search matches highlighted.
- Copy and paste Urdu text.
- Save the file while preserving UTF-8 Urdu text.

---

## 6. Functional Requirements

### FR-1: Open File

**ID:** FR-1  
**Priority:** Must Have

The editor shall allow the user to open a local plain-text file.

**Requirements:**

- The user shall be able to open a file from:
  - A file dialog
  - A command-line argument
- The editor shall open UTF-8 text files.
- The editor shall support files larger than available RAM.
- The editor shall not require loading the entire file into one contiguous memory block.
- If the file cannot be opened, the editor shall show an error message and not crash.

### FR-2: Save File

**ID:** FR-2  
**Priority:** Must Have

The editor shall allow the user to save changes to the same file.

**Requirements:**

- The user shall be able to save using Ctrl+S.
- The editor shall save the file as UTF-8.
- The editor shall overwrite the current file.
- Save As is out of scope.
- If the original file has a UTF-8 BOM, the editor should preserve the BOM.
- If the original file has no BOM, the editor should save without adding a BOM.
- If saving fails due to permissions or file locks, the editor shall show an error message.

### FR-3: Plain-Text Display

**ID:** FR-3  
**Priority:** Must Have

The editor shall display file contents as plain text.

**Requirements:**

- The editor shall render text in a readable font.
- The editor shall support vertical scrolling.
- The editor shall support horizontal scrolling.
- Soft wrap shall be disabled by default.
- The editor shall render only the visible viewport and a small buffer.
- The editor shall remain responsive when displaying very large files.
- The editor shall remain responsive when displaying very long lines.

### FR-4: UTF-8 Handling

**ID:** FR-4  
**Priority:** Must Have

The editor shall correctly handle UTF-8 text.

**Requirements:**

- The editor shall decode UTF-8 text for display, search, copy, paste, and editing.
- The editor shall encode saved text as UTF-8.
- The editor shall support UTF-8 with or without BOM.
- The editor shall not split multi-byte UTF-8 characters during chunked loading or searching.
- The editor shall support Unicode text beyond basic ASCII.
- Invalid UTF-8 bytes shall be handled gracefully.
  - Invalid bytes may be displayed using a replacement character such as U+FFFD.
- The editor shall not crash due to invalid UTF-8 input.

### FR-5: Urdu Text Display

**ID:** FR-5  
**Priority:** Must Have

The editor shall correctly display Urdu text encoded in UTF-8.

**Requirements:**

- The editor shall display Urdu characters correctly.
- The editor shall support Urdu-specific characters, including but not limited to:

  `ٹ`, `ڈ`, `ڑ`, `ں`, `ھ`, `ے`, `ا`, `ب`, `ت`, `ث`, `ج`, `ح`, `خ`, `س`, `ش`, `ص`, `ض`, `ط`, `ظ`, `ع`, `غ`, `ف`, `ق`, `ک`, `گ`, `ل`, `م`, `ن`, `و`, `ہ`, `ی`, `ے`

- Urdu letters shall appear in their correct contextual forms.
- Urdu text shall be displayed with right-to-left reading order.
- The editor shall support basic bidirectional display for mixed Urdu and Latin text.
- The editor shall use a font that supports Urdu glyphs or fall back to a suitable Windows font.
- Urdu text shall not appear as missing-character boxes if a suitable font is available on the system.
- The editor shall use Windows text-rendering capabilities where practical to avoid implementing custom script shaping.

### FR-6: Caret and Selection

**ID:** FR-6  
**Priority:** Must Have

The editor shall support a text caret and text selection.

**Requirements:**

- The user shall be able to move the caret using keyboard arrows.
- The user shall be able to select text using mouse and keyboard.
- The editor shall support selection across:
  - Multiple lines
  - Very long single lines
  - Urdu text
  - Mixed Urdu/Latin text
- Selection shall preserve the logical text order.
- Selection operations shall not require loading the entire file into memory.

### FR-7: Copy

**ID:** FR-7  
**Priority:** Must Have

The editor shall allow the user to copy selected text to the Windows clipboard.

**Requirements:**

- The user shall be able to copy using Ctrl+C.
- Copy shall copy the selected text as Unicode text.
- Copy shall preserve Urdu characters.
- Copy shall preserve UTF-8-compatible Unicode text.
- If no text is selected, Copy shall do nothing.
- Copying large selections shall not freeze the UI.
- If the clipboard operation fails, the editor shall show an error message.

### FR-8: Paste

**ID:** FR-8  
**Priority:** Must Have

The editor shall allow the user to paste text from the Windows clipboard.

**Requirements:**

- The user shall be able to paste using Ctrl+V.
- Paste shall insert clipboard text at the current caret position.
- If text is selected, Paste shall replace the selected text.
- Paste shall accept Unicode text from the clipboard.
- Paste shall preserve Urdu characters.
- Pasted text shall be converted into the editor's internal text representation.
- Paste operations shall be undoable.
- Pasting large text shall not freeze the UI.
- If clipboard content is unavailable or too large, the editor shall fail gracefully.

### FR-9: Basic Text Editing

**ID:** FR-9  
**Priority:** Must Have

The editor shall support minimal plain-text insertion and deletion.

**Requirements:**

- The user shall be able to insert characters using the keyboard.
- The user shall be able to delete selected text.
- The user shall be able to use Backspace and Delete.
- The editor shall support Windows keyboard input.
- The editor shall support Windows IME input where practical.
- Editing shall be limited to plain text.
- All edits shall be undoable and redoable.

### FR-10: Undo

**ID:** FR-10  
**Priority:** Must Have

The editor shall allow the user to undo recent edits.

**Requirements:**

- The user shall be able to undo using Ctrl+Z.
- Undo shall reverse the most recent edit operation.
- Undo shall support at least:
  - Paste
  - Delete
  - Keyboard insertion
- Undo shall restore the previous text state.
- Undo shall restore the caret position reasonably.
- Undo history may be bounded.
- Undo history may be disk-backed for large operations.
- Undo shall not require keeping the entire file in memory.

### FR-11: Redo

**ID:** FR-11  
**Priority:** Must Have

The editor shall allow the user to redo undone edits.

**Requirements:**

- The user shall be able to redo using:
  - Ctrl+Y
  - Ctrl+Shift+Z
- Redo shall reapply the most recently undone operation.
- Redo shall work for the same operations supported by Undo.
- Redo history may be cleared when a new edit is made after an undo.

### FR-12: Search

**ID:** FR-12  
**Priority:** Must Have

The editor shall allow the user to search for text within the open file.

**Requirements:**

- The user shall be able to open search using Ctrl+F.
- The user shall be able to type a search query.
- The search shall support literal substring matching.
- The search shall operate on Unicode text, not raw bytes.
- The search shall support Urdu text queries.
- The search shall support Latin text queries.
- The editor shall support navigating to:
  - Next match
  - Previous match
- Search shall work on large files.
- Search shall not require building a full in-memory index.
- Search shall be incremental or asynchronous.
- Search shall not freeze the UI.
- The user shall be able to close search using Esc.

### FR-13: Case-Sensitive Search Toggle

**ID:** FR-13  
**Priority:** Must Have

The editor shall provide a case-sensitive search toggle.

**Requirements:**

- The search UI shall include a visible case-sensitive toggle.
- When case-sensitive mode is enabled:
  - `abc` shall not match `ABC`
- When case-sensitive mode is disabled:
  - `abc` shall match `ABC`
- The toggle shall apply to Latin-character searches.
- For scripts without case, such as Urdu, the toggle shall have no visible effect.
- The editor may use simple Unicode case folding or at minimum ASCII case folding.
- Full locale-specific case handling is not required unless added later.

### FR-14: Search Result Highlighting

**ID:** FR-14  
**Priority:** Must Have

The editor shall visually highlight search matches.

**Requirements:**

- The editor shall highlight matches in the visible viewport.
- The current match shall be visually distinct from other matches.
- Highlighting shall update when:
  - The search query changes
  - The user scrolls
  - The document is edited
  - The case-sensitive toggle changes
- Highlighting shall support Urdu text.
- Highlighting shall support mixed-direction text.
- Highlighting shall not block the UI.
- Syntax highlighting is explicitly out of scope.

### FR-15: Match Count

**ID:** FR-15  
**Priority:** Must Have

The editor shall display a match count for the current search.

**Requirements:**

- The search UI shall display the number of matches.
- The match count shall update as the search scan progresses.
- For very large files, the count may update incrementally.
- If the total number of matches is extremely large, the editor may display a capped value such as:
  - `99999+`
- The editor shall still allow Next/Previous navigation even if the displayed count is capped.
- If no matches are found, the UI shall show `0` or an equivalent clear indicator.

### FR-16: Scrolling and Navigation

**ID:** FR-16  
**Priority:** Must Have

The editor shall allow smooth navigation through large files.

**Requirements:**

- The editor shall support vertical scrolling.
- The editor shall support horizontal scrolling.
- The editor shall support keyboard navigation:
  - Arrow keys
  - Page Up
  - Page Down
  - Home
  - End
- The editor shall not require loading the entire file to scroll.
- The editor shall remain responsive when scrolling to the beginning or end of large files.
- The editor shall remain responsive when navigating very long lines.

### FR-17: Error Handling

**ID:** FR-17  
**Priority:** Must Have

The editor shall handle errors gracefully.

**Requirements:**

- The editor shall not crash in the following cases:
  - File not found
  - File locked by another process
  - Insufficient permissions
  - Invalid UTF-8 bytes
  - Clipboard unavailable
  - Clipboard content too large
  - Search query too large
  - File exceeds supported limits
  - Font fallback required for Urdu glyphs
- The editor shall display a simple error message when recovery is not possible.

### FR-18: Find and Replace

**ID:** FR-18  
**Priority:** Must Have

The editor shall allow the user to replace occurrences of a search string with a replacement string.

**Requirements:**

- The user shall be able to open replace using Ctrl+H.
- The replace UI shall include:
  - A search input field
  - A replace input field
  - A "Replace Next" button or shortcut
  - A "Replace All" button or shortcut
- Replace shall operate on literal substring matching (no regex).
- Replace shall respect the case-sensitive search toggle.
- Replace shall support Urdu text replacement.
- Replace shall support Latin text replacement.
- Replace shall work on large files without freezing.
- Replace shall be undoable (each replace operation is a single undoable action).
- Replace All shall be undoable as a single undoable action.
- Replace shall not require building a full in-memory index.
- The user shall be able to close replace using Esc.

---

## 7. Non-Functional Requirements

### NFR-1: Lightweight

**Priority:** Must Have

The editor shall be as lightweight as reasonably possible.

**Requirements:**

- Minimal UI
- Minimal dependencies
- No telemetry
- No network access
- No plugin system
- No background services
- Low idle CPU usage
- Small installation footprint

**Suggested Target:**

- Binary or installer size: under 20 MB
- Runtime dependencies: minimal, preferably Windows-native only

### NFR-2: Windows-Native Implementation

**Priority:** Must Have

The editor shall be designed for Windows.

**Suggested Implementation Constraints:**

- Use Windows-native APIs where practical.
- Use DirectWrite or an equivalent Windows text-rendering API for proper Urdu shaping and font fallback.
- Avoid heavy cross-platform UI frameworks unless necessary.
- Avoid bundling large runtime dependencies.

### NFR-3: Large File Support

**Priority:** Must Have

The editor shall handle large files efficiently.

**Minimum Target:**

- File size: at least 1 GiB
- Line count: at least 10 million lines
- Files larger than available RAM

**Stretch Target:**

- File size: 10 GiB or more, depending on hardware

**Requirements:**

- The editor shall not load the entire file into one contiguous memory block.
- The editor shall use chunked loading, memory mapping, or an equivalent technique.
- The editor shall remain usable while opening large files.

### NFR-4: Large Single-Line File Support

**Priority:** Must Have

The editor shall handle files with extremely long single lines.

**Minimum Target:**

- Single line length: at least 100 MiB

**Stretch Target:**

- Single line length: 1 GiB or more

**Requirements:**

- The editor shall not fail because a line is too long.
- The editor shall horizontally virtualize long lines.
- The editor shall render only visible portions of a long line.
- Caret movement, search, copy, and paste shall work within very long lines.
- The editor shall not require wrapping long lines.

### NFR-5: Urdu Rendering Performance

**Priority:** Must Have

The editor shall render Urdu text without excessive performance cost.

**Requirements:**

- The editor shall shape and render only visible text.
- The editor may cache rendered glyph runs for visible or recently visible text.
- The editor shall not shape the entire file at open time.
- For Arabic/Urdu script shaping near viewport boundaries, the editor may load a small number of neighboring characters before and after the visible region to ensure correct joining.

### NFR-6: Memory Usage

**Priority:** Must Have

The editor shall avoid excessive memory consumption.

**Requirements:**

- Memory usage shall not scale linearly with full file size for viewing.
- The editor shall not store the entire file as a single string.
- Undo history shall be bounded or disk-backed.
- Large paste operations shall not require keeping multiple full-file copies in memory.

**Suggested Target:**

- Working set under 300 MB when viewing a 1 GiB file, excluding OS file cache.

### NFR-7: Performance

**Priority:** Must Have

The editor shall remain responsive during normal operations.

**Suggested Performance Targets:**

On a modern Windows PC:

| Operation | Target |
|-----------|--------|
| Application startup | under 1 second |
| Open 1 GiB file | under 3 seconds |
| Scroll latency | under 50 ms for viewport rendering |
| Search first match | under 2 seconds for typical queries |
| Copy 10 MiB selection | under 1 second |
| Paste 10 MiB text | under 1 second |
| Undo/redo of typical operations | under 100 ms |

These targets may vary based on hardware, file structure, and storage speed.

### NFR-8: Reliability

**Priority:** Must Have

The editor shall be stable under large-file and Unicode-text conditions.

**Requirements:**

- The editor shall not crash due to large files.
- The editor shall not crash due to long lines.
- The editor shall not crash due to Urdu text.
- The editor shall not crash due to invalid UTF-8.
- The editor shall degrade gracefully when limits are reached.

### NFR-9: Usability

**Priority:** Should Have

The editor shall be simple to use.

**Requirements:**

- The UI shall contain only essential controls.
- Keyboard shortcuts shall be supported for core operations.
- Search shall be easy to open and close.
- Match count shall be clearly visible.
- Case-sensitive toggle shall be clearly visible.
- Error messages shall be short and understandable.

---

## 8. User Interface Requirements

### 8.1 Main Window

The main window shall contain:

- A text editing viewport
- A minimal search bar
- A simple status or error area
- Match count display

No complex ribbon, toolbar, or menu system is required.

### 8.2 Search/Replace Box

The search/replace box shall include:

- Search input field
- Replace input field
- Next match button or shortcut
- Previous match button or shortcut
- Replace Next button or shortcut
- Replace All button or shortcut
- Case-sensitive toggle
- Match count display
- Close button or Esc support

### 8.3 Keyboard Shortcuts

The editor shall support the following shortcuts:

| Action | Shortcut |
|--------|----------|
| Copy | Ctrl+C |
| Paste | Ctrl+V |
| Undo | Ctrl+Z |
| Redo | Ctrl+Y or Ctrl+Shift+Z |
| Find | Ctrl+F |
| Replace | Ctrl+H |
| Close Find / Replace | Esc |
| Save | Ctrl+S |

---

## 9. Technical Constraints and Design Guidance

### 9.1 Text Buffer Design

The editor should use one of the following:

- Piece table
- Rope
- Chunked buffer
- Memory-mapped file with edit overlay

The editor should avoid:

- Loading the full file into one contiguous string
- Re-copying the full file on every edit
- Building a full line index for the entire file if line count is extremely large

### 9.2 UTF-8 Design

- The editor may use UTF-8 internally for storage efficiency.
- For rendering and search, the editor may temporarily decode portions of text to UTF-16 or Unicode code points.
- The editor shall ensure that multi-byte UTF-8 sequences are not split at chunk boundaries.

### 9.3 Urdu and Right-to-Left Rendering

The editor should use Windows text-rendering support, such as:

- DirectWrite
- Uniscribe or equivalent Windows text-layout support

This is recommended because Urdu uses Arabic-script shaping, contextual letter forms, and right-to-left layout. The editor should avoid implementing custom Urdu shaping unless absolutely necessary.

### 9.4 Search and Replace Design

The editor should:

- Scan text incrementally
- Search in chunks
- Run search asynchronously
- Stop previous searches when the query changes
- Avoid building a full match list for the entire file if the file is huge
- Respect the case-sensitive toggle
- Count matches incrementally
- Perform replace as a piece table delete + insert operation
- Batch Replace All into a single undoable action
- Process Replace All in chunks for large files to avoid UI freezes
- Reuse the search engine's match finding for replace operations

### 9.5 Undo / Redo Design

The editor should:

- Store edit operations as deltas, not full document snapshots
- Use bounded history
- Optionally store large undo data on disk
- Avoid keeping full copies of large pasted content in memory when possible

### 9.6 Encoding Scope

To keep the product lightweight:

- UTF-8 is the required encoding.
- ASCII is supported as a subset of UTF-8.
- Encoding selection UI is out of scope.
- Non-UTF-8 legacy encodings are out of scope unless added later.

### 9.7 Line Endings

The editor should support:

- `LF` (Unix-style)
- `CRLF` (Windows-style)

Line-ending conversion UI is out of scope.

---

## 10. Acceptance Criteria

### 10.1 Basic Functionality

The product shall be considered acceptable if:

- A user can open a local UTF-8 text file.
- A user can select text.
- A user can copy selected text.
- A user can paste clipboard text.
- A user can undo and redo edits.
- A user can search for a substring.
- A user can replace found text.
- Search matches are highlighted.
- Match count is displayed.
- Case-sensitive toggle works.
- A user can save the file.
- The application does not crash during normal use.

### 10.2 Large File Acceptance Test

Given a 1 GiB text file:

- The editor opens the file without crashing.
- The editor scrolls vertically without freezing.
- The editor can move to the end of the file.
- The editor can search for a common substring.
- The editor highlights visible matches.
- The editor displays a match count.
- The case-sensitive toggle changes search behavior.
- The editor can replace text and verify the replacement.
- The editor can copy a 10 MiB selection.
- The editor can paste 10 MiB of text.
- The editor can undo the paste.
- The editor can redo the paste.
- The editor can save the file.
- Memory usage remains within the defined target.

### 10.3 Long Single-Line Acceptance Test

Given a file containing one 100 MiB line:

- The editor opens the file without crashing.
- Horizontal scrolling works.
- The caret can move within the line.
- Search finds matches inside the long line.
- Matches are highlighted in the visible region.
- Match count updates.
- Replace works inside the long line.
- Copying a visible selection works.
- Pasting inside the long line works.
- Undo and redo work for the paste operation.
- Save works without corrupting the file.

### 10.4 UTF-8 Acceptance Test

Given a UTF-8 file containing non-ASCII text:

- The editor opens the file without corrupting characters.
- Non-ASCII characters display correctly.
- Search works for non-ASCII text.
- Copy and paste preserve non-ASCII text.
- Save preserves UTF-8 encoding.
- UTF-8 BOM behavior is handled as specified.

### 10.5 Urdu Acceptance Test

Given a UTF-8 file containing Urdu text such as:

```
اردو زبان کا تجربہ
```

The editor shall:

- Open the file without crashing.
- Display Urdu characters correctly.
- Display Urdu letters in correct joined forms.
- Display Urdu text in right-to-left order.
- Allow selection of Urdu text.
- Allow copying Urdu text.
- Allow pasting Urdu text.
- Allow searching for an Urdu substring such as:
  ```
  زبان
  ```
- Highlight matching Urdu text.
- Display a match count for Urdu search results.
- Allow replacing Urdu text.
- Preserve Urdu text after save and reopen.

### 10.6 Mixed Urdu and Latin Acceptance Test

Given a UTF-8 file containing mixed Urdu and Latin text such as:

```
Hello world — اردو زبان کا تجربہ
```

The editor shall:

- Display both scripts without corruption.
- Display Urdu portions in right-to-left order.
- Display Latin portions in left-to-right order.
- Allow selection across mixed text.
- Allow search across mixed text.
- Allow copy and paste without corrupting either script.

---

## 11. Required MVP Feature Set

The MVP shall include:

- Open file
- Save file
- Plain-text display
- UTF-8 handling
- Urdu text display
- Scrolling
- Caret and selection
- Copy
- Paste
- Basic text editing
- Undo
- Redo
- Search
- Find and replace
- Case-sensitive search toggle
- Search-result highlighting
- Match count
- Minimal error handling

---

## 12. Explicitly Excluded Features

The following shall not be implemented in the initial version:

- Syntax highlighting
- Code folding
- Themes
- Plugins
- Extensions
- Macros
- Scripting
- Multiple tabs
- Multiple windows
- File explorer
- Project/workspace concepts
- Regex search
- Find in files
- Autosave
- Version history
- Cloud sync
- Printing
- Export/conversion
- Spell checking
- Telemetry
- Network access
- macOS support
- Linux support

---

## 13. Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Large files cause high memory usage | High | Use memory-mapped files, chunked buffers, or a piece-table design. Avoid full-file string loading. |
| Very long lines break rendering | High | Use viewport-only rendering and horizontal virtualization. Do not layout the entire line. |
| Urdu rendering requires complex script shaping | Medium | Use Windows-native text rendering such as DirectWrite. Do not implement custom Urdu shaping unless necessary. |
| Search blocks the UI | Medium | Run search asynchronously and incrementally. Cancel old searches when the query changes. |
| Match count is expensive for huge files | Medium | Count matches incrementally and allow capped display such as `99999+`. |
| Undo history consumes too much memory | Medium | Use bounded undo history and store large deltas on disk if needed. |
| Replace All blocks the UI on huge files | Medium | Process Replace All in chunks and batch into a single undo action. Post progress updates asynchronously. |
| Clipboard operations freeze the application | Medium | Perform large copy/paste operations asynchronously and fail gracefully when clipboard limits are exceeded. |

---

## 14. Remaining Open Questions

1. Should the editor support Windows ARM64 in the first release?
2. Should non-UTF-8 encodings be supported later?
3. Should the editor preserve invalid UTF-8 bytes on save, or replace them?
4. Should search support Unicode normalization?
5. Should the editor support full Unicode bidirectional algorithm or only basic RTL display for Urdu?
6. Should the default font be monospaced, or should readability and Urdu support take priority?
7. Should Save create a backup file before overwriting?
8. Should the editor warn before saving over a read-only or locked file?

---

## 15. Final Scope Statement

The product shall be a minimal, lightweight Windows-only plain-text editor designed for opening, editing, searching, replacing, and saving large UTF-8 text files, including files with extremely long lines. It shall support Urdu text display, search highlighting, match count, case-sensitive search, find and replace, copy/paste, undo/redo, and save. It shall prioritize responsiveness, low memory usage, and stability over feature richness.
