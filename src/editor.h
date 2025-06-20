#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <ncurses.h>

class Editor {
public:
    Editor();
    ~Editor();
    
    void run();
    void openFile(const std::string& fname);
    
private:
    // Text buffer - each string is a line
    std::vector<std::string> buffer;
    
    // Cursor position
    int cursorX, cursorY;
    
    // Window dimensions
    int screenRows, screenCols;
    
    // File info
    std::string filename;
    bool isDirty;
    
    // Initialize ncurses
    void initScreen();
    void shutdownScreen();
    
    // Display functions
    void refreshScreen();
    void drawRows();
    void drawStatusBar();
    void updateCursor();
    
    // Input handling
    void handleKeypress();
    void moveCursor(int key);
    
    // File operations
    void saveFile();
    
    // Editing operations
    void insertChar(char c);
    void deleteChar();
    void insertNewline();
};

#endif // EDITOR_H 