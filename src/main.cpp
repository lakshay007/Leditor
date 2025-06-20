#include "editor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Editor editor;
    
    // If a filename is provided, try to open it
    if (argc > 1) {
        editor.openFile(argv[1]);
    }
    
    // Run the editor
    editor.run();
    
    return 0;
} 