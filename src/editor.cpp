#include <editor.hh>
#include <keys.hh>
#include <errmgr.hh>
#include <termgui.hh>
#include <fileio.hh>
#include <alert.hh>
#include <unistd.h>

Editor::Editor(Config &cfg) : config(cfg) {};

void Editor::moveCursor(int c) {
    std::shared_ptr<Row> data = config.cy >= config.fileData.size() ? nullptr : config.fileData.at(config.cy);
    
    switch(c) {
        case ARROW_UP:
            if (config.cy > 0) config.cy--;
            break;
        case ARROW_LEFT:
            if (config.cx != 0) {
                config.cx--;
            } else if (config.cy > 0) {
                config.cy--;
                config.cx = config.fileData.at(config.cy)->sRaw.size();
            }
            break;
        case ARROW_DOWN:
            if (config.cy < config.fileData.size()) config.cy++;
            break;
        case ARROW_RIGHT:
            if (data && config.cx < data->sRaw.size()) {
                config.cx++;
            } else if (data && config.cx == data->sRaw.size()) {
                config.cy++;
                config.cx = 0;
            }
            break;
    }

    data = config.cy >= config.fileData.size() ? nullptr : config.fileData.at(config.cy);
    size_t newlen = data ? data->sRaw.size() : 0;
    if (config.cx > newlen) {
        config.cx = newlen;
    }
}

int Editor::processKey(bool breakAny) {
    static int quitStroke = 1;
    int c = this->readKey();
    if (breakAny) return 0;
    switch (c) {
        // TODO: Newline
        case '\r':
            insertNewLine();
            break;
        case K_CTRL('q'): {
            // if (config.modified && quitStroke > 0) {
            //     Alert::setStatusMsg(this->config, "UNSAVED CHANGES | Press quit again to discard changes");
            //     quitStroke--;
            //     break;
            // }
            return -1;
        }

        case K_CTRL('s'):
            if (1 == FileIO::saveFile(this->config)) {
                // TODO: error Checking
            }
            break;

        case PAGE_UP:
        case PAGE_DOWN: {
            if (c == PAGE_UP) {
                config.cy = config.rOffset;
            } else if (c == PAGE_DOWN) {
                config.cy = config.rOffset + config.sRow - 1;
                if (config.cy > config.fileData.size()) {
                    config.cy = config.fileData.size();
                }
            }
            
            int times = config.sRow;
            while (times--) {
                moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        }

        case HOME:
            config.cx = 0;
            break;
        case END:
            if (config.cy < config.fileData.size()) {
                config.cx = config.fileData.at(config.cy)->sRaw.size();
            }
            break;

        // TODO: implement
        case BACKSPACE:
        case K_CTRL('h'):
        case DEL:
            if (c == DEL) moveCursor(ARROW_RIGHT); // delete character to the right of cursor currently
            delChar();
            break;

        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            moveCursor(c);
            break;

        // Ignore for now
        case K_CTRL('l'):
        case '\x1b':
            break;

        default:
            // handle other keys
            insertChar(c);
            break;
    }

    quitStroke = 1;
    return 0;
}