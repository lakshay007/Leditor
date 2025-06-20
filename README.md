# Leditor - Lightweight Text Editor

A basic text editor written in C++ with both terminal and GUI versions.

## Versions

### 1. Terminal Version (ncurses)
A vim-like terminal-based text editor using ncurses.

### 2. GUI Version (Qt)
A modern GUI text editor with normal cursor, menus, and Notepad-like interface.

## Prerequisites

- **macOS/Linux**: C++17 compiler (usually pre-installed)
- **GUI Version**: Qt6 - Install with `brew install qt`
- **Terminal Version**: ncurses library (usually pre-installed on macOS/Linux)

## Quick Start for New Users

### Build and Run GUI Version (Recommended)
```bash
make gui-run          # Builds and launches GUI version
```

### Build and Run Terminal Version
```bash
make run-terminal     # Builds and launches terminal version
```

### Build Everything
```bash
make build-all        # Builds both terminal and GUI versions
```

## Detailed Building

### Terminal Version
```bash
make                  # Build terminal version
./leditor             # Run terminal version
```

### GUI Version
```bash
make gui              # Build GUI version
./leditor-gui         # Run GUI version
```

## Usage

### Terminal Version
```bash
./leditor [filename]  # Optional filename to open
```

### GUI Version
```bash
./leditor-gui [filename]  # Optional filename to open
```

## Features

### Terminal Version
- **Arrow Keys** - Navigate through text
- **Backspace** - Delete character before cursor
- **Enter** - Insert new line
- **Ctrl+S** - Save file
- **Ctrl+Q** - Quit (press twice if there are unsaved changes)
- Any printable character - Insert at cursor position

### GUI Version
- **Normal text editing** - Click to position cursor, type to insert
- **File Menu** - New, Open, Save, Save As, Exit
- **Standard shortcuts** - Cmd+N (New), Cmd+O (Open), Cmd+S (Save)
- **Status bar** - Shows filename, modified status, and cursor position
- **About dialog** - Help menu with application info
- **Automatic text wrapping**
- **Monaco monospace font** for clean display

## Requirements

- **Terminal Version**: C++17 compiler, ncurses library
- **GUI Version**: C++17 compiler, Qt6 (installed via Homebrew)

## Architecture

### Terminal Version
- **editor.h/cpp** - Main editor class handling ncurses UI and logic
- **main.cpp** - Entry point
- Uses `std::vector<std::string>` for text storage

### GUI Version
- **mainwindow.h/cpp** - Main window with menus, file operations, status bar
- **texteditor.h/cpp** - Custom QTextEdit widget with cursor tracking
- **main.cpp** - Qt application entry point
- Uses Qt's built-in text document model

## Next Steps

Potential enhancements for both versions:
- Syntax highlighting
- Find/Replace functionality
- Undo/Redo system
- Multiple tabs/buffers
- Line numbers
- Better file encoding support 