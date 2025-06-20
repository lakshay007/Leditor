#include "editor.h"
#include <fstream>
#include <iostream>
#include <algorithm>

Editor::Editor() : cursorX(0), cursorY(0), screenRows(0), screenCols(0), isDirty(false) {
    initScreen();
    
    // one empty line if buffer is empty
    if (buffer.empty()) {
        buffer.push_back("");
    }
}

Editor::~Editor() {
    shutdownScreen();
}

void Editor::initScreen() {
    // Initialize ncurses
    initscr();
    raw();              // Disable line buffering
    keypad(stdscr, TRUE); // Enable special keys
    noecho();           // Don't echo key presses
    
    // Get terminal size
    getmaxyx(stdscr, screenRows, screenCols);
    
    // Reserve last line for status bar
    screenRows--;
}

void Editor::shutdownScreen() {
    endwin();
}

void Editor::run() {
    while (true) {
        refreshScreen();
        handleKeypress();
    }
}

void Editor::refreshScreen() {
    clear();
    drawRows();
    drawStatusBar();
    updateCursor();
    refresh();
}

void Editor::drawRows() {
    for (int y = 0; y < screenRows; y++) {
        move(y, 0);
        
        if (y < (int)buffer.size()) {
            // Draw text line
            std::string line = buffer[y];
            if ((int)line.length() > screenCols) {
                line = line.substr(0, screenCols);
            }
            printw("%s", line.c_str());
        } else {
            // Draw ~ for empty lines like vim hehehe
            printw("~");
        }
        
        clrtoeol(); // Clear to end of line
    }
}

void Editor::drawStatusBar() {
    // Move to last line
    move(screenRows, 0);
    
    // Reverse video for status bar
    attron(A_REVERSE);
    
    // Status info
    std::string status = filename.empty() ? "[No Name]" : filename;
    if (isDirty) status += " [Modified]";
    
    // Position info
    char posInfo[80];
    snprintf(posInfo, sizeof(posInfo), "Line %d, Col %d", cursorY + 1, cursorX + 1);
    
    // Draw status
    printw("%-*s %s", screenCols - strlen(posInfo) - 1, status.c_str(), posInfo);
    
    attroff(A_REVERSE);
}

void Editor::updateCursor() {
    move(cursorY, cursorX);
}

void Editor::handleKeypress() {
    int c = getch();
    
    switch (c) {
        case 'q' - 'a' + 1: // Ctrl-Q to quit
            if (!isDirty || (getch() == 'q' - 'a' + 1)) {
                shutdownScreen();
                exit(0);
            }
            break;
            
        case 's' - 'a' + 1: // Ctrl-S to save
            saveFile();
            break;
            
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            moveCursor(c);
            break;
            
        case KEY_BACKSPACE:
        case 127:
        case 8:
            deleteChar();
            break;
            
        case KEY_ENTER:
        case '\n':
        case '\r':
            insertNewline();
            break;
            
        default:
            if (c >= 32 && c < 127) { // Printable characters
                insertChar(c);
            }
            break;
    }
}

void Editor::moveCursor(int key) {
    switch (key) {
        case KEY_LEFT:
            if (cursorX > 0) {
                cursorX--;
            } else if (cursorY > 0) {
                cursorY--;
                cursorX = buffer[cursorY].length();
            }
            break;
            
        case KEY_RIGHT:
            if (cursorY < (int)buffer.size() && cursorX < (int)buffer[cursorY].length()) {
                cursorX++;
            } else if (cursorY < (int)buffer.size() - 1) {
                cursorY++;
                cursorX = 0;
            }
            break;
            
        case KEY_UP:
            if (cursorY > 0) {
                cursorY--;
                cursorX = std::min(cursorX, (int)buffer[cursorY].length());
            }
            break;
            
        case KEY_DOWN:
            if (cursorY < (int)buffer.size() - 1) {
                cursorY++;
                cursorX = std::min(cursorX, (int)buffer[cursorY].length());
            }
            break;
    }
}

void Editor::insertChar(char c) {
    if (cursorY < (int)buffer.size()) {
        buffer[cursorY].insert(cursorX, 1, c);
        cursorX++;
        isDirty = true;
    }
}

void Editor::deleteChar() {
    if (cursorX > 0 && cursorY < (int)buffer.size()) {
        buffer[cursorY].erase(cursorX - 1, 1);
        cursorX--;
        isDirty = true;
    } else if (cursorX == 0 && cursorY > 0) {
        // Join with previous line
        cursorX = buffer[cursorY - 1].length();
        buffer[cursorY - 1] += buffer[cursorY];
        buffer.erase(buffer.begin() + cursorY);
        cursorY--;
        isDirty = true;
    }
}

void Editor::insertNewline() {
    if (cursorY < (int)buffer.size()) {
        std::string currentLine = buffer[cursorY];
        std::string newLine = currentLine.substr(cursorX);
        buffer[cursorY] = currentLine.substr(0, cursorX);
        buffer.insert(buffer.begin() + cursorY + 1, newLine);
        cursorY++;
        cursorX = 0;
        isDirty = true;
    }
}

void Editor::openFile(const std::string& fname) {
    std::ifstream file(fname);
    if (file.is_open()) {
        buffer.clear();
        std::string line;
        while (std::getline(file, line)) {
            buffer.push_back(line);
        }
        file.close();
        
        if (buffer.empty()) {
            buffer.push_back("");
        }
        
        filename = fname;
        isDirty = false;
        cursorX = 0;
        cursorY = 0;
    }
}

void Editor::saveFile() {
    if (filename.empty()) {
        // save as "untitled.txt" for now ughh
        filename = "untitled.txt";
    }
    
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& line : buffer) {
            file << line << '\n';
        }
        file.close();
        isDirty = false;
    }
} 