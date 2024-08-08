#include <editor.hh>
#include <errmgr.hh>
#include <termgui.hh>
#include <fileio.hh>
#include <unistd.h>
#include <inreader.hh>

Editor::Editor(Config &cfg) : config(cfg) {};

void Editor::moveCursor(int c) {
    std::shared_ptr<Row> data = config.cursor.cy >= config.fileData.size() ? nullptr : config.fileData.at(config.cursor.cy);
    
    switch(c) {
        case ARROW_UP:
            if (config.cursor.cy > 0) config.cursor.cy--;
            break;
        case ARROW_LEFT:
            if (config.cursor.cx != 0) {
                config.cursor.cx--;
            } else if (config.cursor.cy > 0) {
                config.cursor.cy--;
                config.cursor.cx = config.fileData.at(config.cursor.cy)->sRaw.size();
            }
            break;
        case ARROW_DOWN:
            if (config.cursor.cy < config.fileData.size()) config.cursor.cy++;
            break;
        case ARROW_RIGHT:
            if (data && config.cursor.cx < data->sRaw.size()) {
                config.cursor.cx++;
            } else if (data && config.cursor.cx == data->sRaw.size()) {
                config.cursor.cy++;
                config.cursor.cx = 0;
            }
            break;
    }

    data = config.cursor.cy >= config.fileData.size() ? nullptr : config.fileData.at(config.cursor.cy);
    size_t newlen = data ? data->sRaw.size() : 0;
    if (config.cursor.cx > newlen) {
        config.cursor.cx = newlen;
    }
}

int Editor::processKey(bool breakAny) {
    static int quitStroke = 1;
    int c = InputReader::readKey();
    if (breakAny) return 0;
    switch (c) {
        // TODO: Newline
        case '\r':
            config.fileData.insertNewLine(config.cursor);
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
            if (1 == FileIO::saveFile(config.fileData)) {
                // TODO: error Checking
            }
            break;

        case PAGE_UP:
        case PAGE_DOWN: {
            if (c == PAGE_UP) {
                config.cursor.cy = config.cursor.rOffset;
            } else if (c == PAGE_DOWN) {
                config.cursor.cy = config.cursor.rOffset + config.term.sRow - 1;
                if (config.cursor.cy > config.fileData.size()) {
                    config.cursor.cy = config.fileData.size();
                }
            }
            
            int times = config.term.sRow;
            while (times--) {
                moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        }

        case HOME:
            config.cursor.cx = 0;
            break;
        case END:
            if (config.cursor.cy < config.fileData.size()) {
                config.cursor.cx = config.fileData.at(config.cursor.cy)->sRaw.size();
            }
            break;

        // TODO: implement
        case BACKSPACE:
        case K_CTRL('h'):
        case DEL:
            if (c == DEL) moveCursor(ARROW_RIGHT); // delete character to the right of cursor currently
            config.fileData.deleteChar(config.cursor);
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
            config.fileData.insertChar(config.cursor, c);
            break;
    }

    quitStroke = 1;
    return 0;
}