#include <termgui.hh>
#include <version.hh>
#include <sys/ioctl.h>
#include <errmgr.hh>
#include <ctime>
#include <unistd.h>
#include <termacts.hh>
#include <iostream>
#include <inhandler.hh>

#define CURSOR_X_SHIFT 3

TerminalGUI::TerminalGUI(Config &cfg) : config(cfg) {}

void TerminalGUI::flushBuf()
{
    std::string s = buf.str();
    write(STDOUT_FILENO, s.c_str(), s.size());
    buf.str("");
}

void TerminalGUI::updateCursor(const TTEdCursor &cursor)
{
    buf << "\x1b[" << (cursor.cy - cursor.rOffset) + 1 << ";" << cursor.rx + CURSOR_X_SHIFT << "H";
}

void TerminalGUI::drawRows(const TTEdCursor &cursor, const TTEdFileData &fData, const TTEdTermData &tData)
{
    for (size_t r = 0; r < tData.sRow; ++r)
    {
        size_t rowLoc = r + cursor.rOffset;

        if (rowLoc >= fData.size())
        {
            buf << "~\x1b[K\r\n";
        }
        else
        {
            auto row = fData.at(rowLoc);

            auto start = row->sRender.cbegin();
            auto end = row->sRender.cend();

            if (cursor.cOffset < row->size())
            {
                start += cursor.cOffset;
            }

            size_t shift = cursor.cOffset + tData.sCol;
            if (row->sRender.size() > shift)
            {
                end = start + shift;
            }

            buf << "~ ";
            int current_color = -1;
            for (size_t i = 0; i < row->size(); i++) {
                textState state = row->textStates.at(i);
                if (state == TS_NORMAL) {
                    if (current_color != -1) {
                        buf << "\x1b[m";
                        buf << "\x1b[39m";
                        current_color = -1;
                    }   
                    buf << row->sRender.at(i);
                }
                else {
                    int color = this->stateToColor.at(state);
                    if (current_color != color) {
                        current_color = color;
                        buf << "\x1b[" << std::to_string(color) << "m";
                    }
                    buf << row->sRender.at(i);
                }
            }
            buf << "\x1b[39m\x1b[K\r\n";
        }
    }
}

void TerminalGUI::drawStatusBar(const Config &cfg)
{
    buf << "\x1b[7m";

    std::string leftStatus = cfg.fileData.filename + " - " + std::to_string(cfg.fileData.size()) + " lines";

    std::string fileType = (cfg.syntax != NULL ) ? cfg.syntax->filetype : "?";
    std::string connectionStatus = (cfg.conn.connected) ? cfg.conn.host ? "(host)" : "(remote)" : "";
    std::string rightStatus = connectionStatus + " | " + fileType + " | " + std::to_string(cfg.cursor.cy + 1) + "," + std::to_string(cfg.cursor.cx + 1);
    if (cfg.fileData.modified > 0)
    {
        rightStatus += " M";
    }

    std::string spaces(cfg.term.sCol - rightStatus.size() - leftStatus.size(), ' ');

    buf << leftStatus << spaces << rightStatus;
    buf << "\x1b[m\r\n";
}

void TerminalGUI::drawMessageBar(const TTEdTermData &tData, const TTEdStatus &status)
{
    buf << "\x1b[K";
    int msgLen = std::min(status.statusMsg.size(), tData.sCol);
    if (msgLen && (std::time(nullptr) - status.statusTime) < 5)
    {
        buf << status.statusMsg.substr(0, msgLen);
    }
}

std::string TerminalGUI::centerText(const Config &config, const std::string &s)
{
    if (s.size() > config.term.sCol)
        return s.substr(0, config.term.sCol);
    std::string spaces((config.term.sCol - s.size()) / 2, ' ');
    return spaces + s;
}

void TerminalGUI::genCoverPage(const Config &config, std::stringstream &s)
{
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
    v.push_back(centerText(config, "(Press any key to continue)"));
    v.push_back("\e[0m");

    size_t verticalPadding = (config.term.sRow - v.size()) / 2;
    for (size_t i = 0; i < verticalPadding + v.size(); i++)
    {
        s << (i < verticalPadding ? "\n" : v.at(i - verticalPadding));
    }
}

void TerminalGUI::draw()
{
    TermActions::hideCursor(buf);
    TermActions::wipeScreen(buf);
    TermActions::resetCursor(buf);
    config.scroll();
    drawRows(config.cursor, config.fileData, config.term);
    drawStatusBar(config);
    drawMessageBar(config.term, config.status);
    updateCursor(config.cursor);
    TermActions::showCursor(buf);
    flushBuf();
}

void TerminalGUI::splashScreen()
{
    genCoverPage(config, buf);
    flushBuf();
    InputHandler::processKey(config.cursor, config.fileData, config.term, config.status, true);
}

void TerminalGUI::reset()
{
    TermActions::wipeScreen(buf);
    TermActions::resetCursor(buf);
    flushBuf();
}
