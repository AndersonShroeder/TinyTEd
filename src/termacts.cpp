#include <termacts.hh>

void TermActions::wipeScreen(std::stringstream &buf) {
    buf << "\x1b[2J";
}

void TermActions::resetCursor(std::stringstream &buf) {
    buf << "\x1b[H";
}

void TermActions::hideCursor(std::stringstream &buf) {
    buf << "\x1b[?25l";
}

void TermActions::showCursor(std::stringstream &buf) {
    buf << "\x1b[?25h";
}