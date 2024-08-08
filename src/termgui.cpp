#include <termgui.hh>
#include <version.hh>
#include <sys/ioctl.h>
#include <errmgr.hh>
#include <ctime>
#include <unistd.h>
#include <termacts.hh>
#include <iostream>
#include <inhandler.hh>

TerminalGUI::TerminalGUI(Config &cfg) : config(cfg) {};

void TerminalGUI::flushBuf() {
    std::string s = buf.str();
    write(STDOUT_FILENO, s.c_str(), s.size());
    buf.str("");
}

void TerminalGUI::updateCursor(TTEdCursor &cursor) {
    std::stringstream ss;
    ss << "\x1b[" << (cursor.cy - cursor.rOffset) + 1 << ";" << cursor.rx + 1 << "H";
    buf << ss.str();
}

void TerminalGUI::drawRows(TTEdCursor &cursor, TTEdFileData fData, TTEdTermData tData) {
    for (size_t r = 0; r < tData.sRow; r++) {
        size_t rowLoc = r + cursor.rOffset;

        if (rowLoc >= fData.size()) {
            buf << "~\x1b[K\r\n";
        }
        
        else {
            std::shared_ptr<Row> row = fData.at(rowLoc);

            // Get string as iterators so we can modify starting/ending point to render
            std::string::const_iterator start, end;
            start = row->sRender.cbegin();
            end = row->sRender.cend();

            if (cursor.cOffset < row->size()) {
                start += cursor.cOffset;
            }
            
            size_t shift = cursor.cOffset + tData.sCol;
            if (row->sRender.size() > (shift)) {
                end = start + shift;
            }

            buf << std::string(start, end) << "\x1b[K\r\n";
        }
    }
}

void TerminalGUI::drawStatusBar(TTEdCursor &cursor, TTEdFileData &fData, TTEdTermData &tData) {
    buf << "\x1b[7m";

    std::stringstream ss;

    std::string leftStatus = fData.filename + " - " + std::to_string(fData.size()) + " lines";
    std::string rightStatus = std::to_string(cursor.cy + 1) + "," + std::to_string(cursor.cx + 1);
    if (fData.modified > 0) {
        rightStatus += " M";
    }

    std::string spaces(tData.sCol - rightStatus.size() - leftStatus.size(), ' ');

    ss << leftStatus << spaces << rightStatus;

    buf << ss.str().substr(0, tData.sCol) << "\x1b[m\r\n";
}

void TerminalGUI::drawMessageBar(TTEdTermData tData, TTEdStatus status) {
    buf << "\x1b[K";
    int msgLen = std::min(status.statusMsg.size(), tData.sCol);
    if (msgLen && (std::time(nullptr) - status.statusTime) < 5) {
        buf << status.statusMsg.substr(0, msgLen);
    }
}

std::string TerminalGUI::centerText(Config& config, std::string s) {
    if (s.size() > config.term.sCol) return s.substr(0, config.term.sCol);
    std::stringstream ss;
    size_t lpadding = (config.term.sCol - s.size()) / 2;
    while (lpadding--) ss << " ";
    ss << s;
    return ss.str();
}

void TerminalGUI::genCoverPage(Config& config, std::string &s) {
    std::vector<std::string> v;
    v.push_back("\033[1;36m");
    v.push_back(centerText(config, "   ___                       ___           ___           ___           ___           ___     \r\n"));
    v.push_back(centerText(config, "  /\\  \\          ___        /\\__\\         |\\__\\         /\\  \\         /\\  \\         /\\  \\    \r\n"));
    v.push_back(centerText(config, "  \\:\\  \\        /\\  \\      /::|  |        |:|  |        \\:\\  \\       /::\\  \\       /::\\  \\   \r\n"));
    v.push_back(centerText(config, "   \\:\\  \\       \\:\\  \\    /:|:|  |        |:|  |         \\:\\  \\     /:/\\:\\  \\     /:/\\:\\  \\  \r\n"));
    v.push_back(centerText(config, "   /::\\  \\      /::\\__\\  /:/|:|  |__      |:|__|__       /::\\  \\   /::\\~\\:\\  \\   /:/  \\:\\__\\ \r\n"));
    v.push_back(centerText(config, "  /:/\\:\\__\\  __/:/\\/__/ /:/ |:| /\\__\\     /::::\\__\\     /:/\\:\\__\\ /:/\\:\\ \\:\\__\\ /:/__/ \\:|__|\r\n"));
    v.push_back(centerText(config, " /:/  \\/__/ /\\/:/  /    \\/__|:|/:/  /    /:/~~/~       /:/  \\/__/ \\:\\~\\:\\ \\/__/ \\:\\  \\ /:/  /\r\n"));
    v.push_back(centerText(config, "/:/  /      \\::/__/         |:/:/  /    /:/  /        /:/  /       \\:\\ \\:\\__\\    \\:\\  /:/  / \r\n"));
    v.push_back(centerText(config, "\\/__/        \\:\\__\\         |::/  /     \\/__/         \\/__/         \\:\\ \\/__/     \\:\\/:/  /  \r\n"));
    v.push_back(centerText(config, "              \\/__/         /:/  /                                   \\:\\__\\        \\::/__/   \r\n"));
    v.push_back(centerText(config, "                            \\/__/                                     \\/__/         ~~       \r\n"));
    v.push_back("\n\n");
    v.push_back("\e[1;35m");
    v.push_back(centerText(config, "A Minimalist Command Line Text Editor\r\n"));
    std::string tmp = "Version ";
    tmp += VERSION;
    tmp += "\r\n";
    v.push_back(centerText(config, tmp));
    v.push_back(centerText(config, "(Press any key to continue)\r\n"));
    v.push_back("\e[0m");

    size_t verticalPadding = (config.term.sRow - v.size()) / 2;
    for (size_t i = 0; i < verticalPadding + v.size(); i++) {
        s += i < verticalPadding ? "\n" : v.at(i - verticalPadding);
    }
}

void TerminalGUI::draw() {
    TermActions::hideCursor(buf);
    TermActions::wipeScreen(buf);
    TermActions::resetCursor(buf);
    config.scroll();
    drawRows(config.cursor, config.fileData, config.term);
    drawStatusBar(config.cursor, config.fileData, config.term);
    drawMessageBar(config.term, config.status);
    updateCursor(config.cursor);
    TermActions::showCursor(buf);
    flushBuf();
}

void TerminalGUI::splashScreen() {
    std::string s;
    genCoverPage(config, s);
    write(STDOUT_FILENO, s.c_str(), s.size());
    InputHandler::processKey(config.cursor, config.fileData, config.term);
}

void TerminalGUI::reset() {
    TermActions::wipeScreen(buf);
    TermActions::resetCursor(buf);
    flushBuf();
}
