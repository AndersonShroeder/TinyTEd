#include <termgui.hh>
#include <version.hh>
#include <sys/ioctl.h>
#include <errmgr.hh>
#include <alert.hh>
#include <ctime>

TerminalGUI::TerminalGUI(Config &cfg) : config(cfg) {};

void TerminalGUI::wipeScreen() {
    buf << "\x1b[2J";
}

void TerminalGUI::resetCursor() {
    buf << "\x1b[H";
}

void TerminalGUI::hideCursor() {
    buf << "\x1b[?25l";
}

void TerminalGUI::showCursor() {
    buf << "\x1b[?25h";
}

int TerminalGUI::rowCxToRx(std::shared_ptr<Row> row) {
    int rx = 0;
    for (size_t j = 0; j < config.cx; j++) {
        if (row->sRaw.at(j) == '\t')
            rx += (TABSTOP - 1) - (rx % TABSTOP);
        rx++;
    }

    return rx;
}

void TerminalGUI::scroll() {
    config.rx = 0;
    if (config.cy < config.fileData.size()) {
        config.rx = rowCxToRx(config.fileData.at(config.cy));
    }

    if (config.cy < config.rOffset) config.rOffset = config.cy;
    if (config.cy >= config.rOffset + config.sRow) config.rOffset = config.cy - config.sRow + 1;

    if (config.rx < config.cOffset) config.cOffset = config.rx;
    if (config.rx >= config.cOffset + config.sCol) config.cOffset = config.rx - config.sCol + 1;
}

void TerminalGUI::flushBuf() {
    std::string s = buf.str();
    write(STDOUT_FILENO, s.c_str(), s.size());
    buf.str("");
}

void TerminalGUI::updateCursor() {
    std::stringstream ss;
    ss << "\x1b[" << (config.cy - config.rOffset) + 1 << ";" << config.rx + 1 << "H";
    buf << ss.str();
}

void TerminalGUI::drawRows() {
    for (size_t r = 0; r < config.sRow; r++) {
        if (r + config.rOffset >= config.fileData.size()) {
            buf << "~\x1b[K\r\n";
        } else {
            std::string::const_iterator start = config.fileData.at(r + config.rOffset)->sRender.cbegin();
            std::string::const_iterator end = config.fileData.at(r + config.rOffset)->sRender.cend();

            if (config.cOffset < config.fileData.at(r + config.rOffset)->sRender.size()) {
                start += config.cOffset;
            }
            if (config.fileData.at(r + config.rOffset)->sRender.size() > (config.cOffset + config.sCol)) {
                end = config.fileData.at(r + config.rOffset)->sRender.cbegin() + (config.cOffset + config.sCol);
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

    std::string leftStatus = config.filename + " - " + std::to_string(config.fileData.size()) + " lines";
    // {row}, {col} {modified?}
    std::string rightStatus = std::to_string(config.cy + 1) + "," + std::to_string(config.cx + 1);
    rightStatus += config.modified > 0 ? " M" : "";
    ss << leftStatus;

    while (ss.str().size() < config.sCol - rightStatus.size()) {
        ss << " ";
    }
    ss << rightStatus;

    buf << ss.str().substr(0, config.sCol);
    buf << "\x1b[m";
    buf << "\r\n";
}

void TerminalGUI::drawMessageBar() {
    buf << "\x1b[K";
    int msgLen = std::min(config.statusMsg.size(), config.sCol);
    if (msgLen && (std::time(nullptr) - config.statusTime) < 5) {
        buf << config.statusMsg.substr(0, msgLen);
    }
}

std::string TerminalGUI::centerText(Config& config, std::string s) {
    if (s.size() > config.sCol) return s.substr(0, config.sCol);
    std::stringstream ss;
    size_t lpadding = (config.sCol - s.size()) / 2;
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

    size_t verticalPadding = (config.sRow - v.size()) / 2;
    for (size_t i = 0; i < verticalPadding + v.size(); i++) {
        s += i < verticalPadding ? "\n" : v.at(i - verticalPadding);
    }
}

void TerminalGUI::draw() {
    hideCursor();
    wipeScreen();
    resetCursor();
    scroll();
    drawRows();
    drawStatusBar();
    drawMessageBar();
    updateCursor();
    showCursor();
    flushBuf();
}

void TerminalGUI::splashScreen(Editor &e) {
    std::string s;
    genCoverPage(config, s);
    write(STDOUT_FILENO, s.c_str(), s.size());
    e.processKey();
}

int TerminalGUI::getWindowSize(Config& config) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        ErrorMgr::err("unable to retrieve window size");
    } else {
        config.sCol = ws.ws_col;
        config.sRow = ws.ws_row - 2;
        return 0;
    }
    return -1;
}

int TerminalGUI::getCursorPosition(Config& config) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%zu;%zu", &config.sRow, &config.sCol) != 2) return -1;
    return 0;
}

void TerminalGUI::initGUI() {
    if (getWindowSize(config) == -1) ErrorMgr::err("window size error");
}

void TerminalGUI::reset() {
    wipeScreen();
    resetCursor();
    flushBuf();
}