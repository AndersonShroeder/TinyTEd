#include <termgui.hh>
#include <version.hh>
#include <sys/ioctl.h>
#include <errmgr.hh>
#include <ctime>
#include <unistd.h>
#include <termacts.hh>
#include <iostream>

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
        if (r + cursor.rOffset >= fData.size) {
            buf << "~\x1b[K\r\n";
        } else {
            std::string::const_iterator start = fData.at(r + cursor.rOffset)->sRender.cbegin();
            std::string::const_iterator end = fData.at(r + cursor.rOffset)->sRender.cend();

            if (cursor.cOffset < fData.at(r + cursor.rOffset)->sRender.size()) {
                start += cursor.cOffset;
            }
            if (fData.at(r + cursor.rOffset)->sRender.size() > (cursor.cOffset + tData.sCol)) {
                end = fData.at(r + cursor.rOffset)->sRender.cbegin() + (cursor.cOffset + tData.sCol);
            }

            buf << std::string(start, end);
            buf << "\x1b[K";
            buf << "\r\n";
        }
    }
}

void TerminalGUI::drawStatusBar() {
    buf << "\x1b[7m";
    std::stringstream ss;

    std::string leftStatus = config.fileData.filename + " - " + std::to_string(config.fileData.size) + " lines";
    // {row}, {col} {modified?}
    std::string rightStatus = std::to_string(config.cursor.cy + 1) + "," + std::to_string(config.cursor.cx + 1);
    rightStatus += config.fileData.modified > 0 ? " M" : "";
    ss << leftStatus;

    while (ss.str().size() < config.term.sCol - rightStatus.size()) {
        ss << " ";
    }
    ss << rightStatus;

    buf << ss.str().substr(0, config.term.sCol);
    buf << "\x1b[m";
    buf << "\r\n";
}

void TerminalGUI::drawMessageBar() {
    buf << "\x1b[K";
    int msgLen = std::min(config.status.statusMsg.size(), config.term.sCol);
    if (msgLen && (std::time(nullptr) - config.status.statusTime) < 5) {
        buf << config.status.statusMsg.substr(0, msgLen);
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
    drawStatusBar();
    drawMessageBar();
    updateCursor(config.cursor);
    TermActions::showCursor(buf);
    flushBuf();
}

void TerminalGUI::splashScreen(Editor &e) {
    std::string s;
    genCoverPage(config, s);
    write(STDOUT_FILENO, s.c_str(), s.size());
    e.processKey();
}

void TerminalGUI::reset() {
    TermActions::wipeScreen(buf);
    TermActions::resetCursor(buf);
    flushBuf();
}