#include <termacts.hh>

void TermActions::wipeScreen(std::stringstream &buf) {
    // Escape sequence to clear the screen
    buf << "\x1b[2J";
}

void TermActions::resetCursor(std::stringstream &buf) {
    // Escape sequence to reset cursor to home position
    buf << "\x1b[H";
}

void TermActions::hideCursor(std::stringstream &buf) {
    // Escape sequence to hide the cursor
    buf << "\x1b[?25l";
}

void TermActions::showCursor(std::stringstream &buf) {
    // Escape sequence to show the cursor
    buf << "\x1b[?25h";
}
