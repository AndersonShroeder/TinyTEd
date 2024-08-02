#include <editor.hh>
#include <keys.hh>
#include <errmgr.hh>
#include <termgui.hh>
#include <fileio.hh>
#include <alert.hh>

Editor::Editor(Config &cfg) : config(cfg) {};

int Editor::readKey() {
    char c;
    while ((r = read(STDIN_FILENO, &c, sizeof(c))) != 1) {
        if (r < 0 && errno != EAGAIN) {
            ErrorMgr::err("read");
        }
    }

    if (c == '\x1b') {
        std::array<char, 3> buf;
        if (read(STDIN_FILENO, &buf[0], 1) != 1) return c;
        if (read(STDIN_FILENO, &buf[1], 1) != 1) return c;
        
        if (buf[0] == '[') {
            if (buf[1] >= '0' && buf[1] <= '9') {
                if (read(STDIN_FILENO, &buf[2], 1) != 1) return c;
                if (buf[2] == '~') {
                    switch (buf[1]) {
                        case '1': return HOME;
                        case '3': return DEL;
                        case '4': return END;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME;
                        case '8': return END;
                    }
                }

            } else {
                switch (buf[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME;
                    case 'F': return END;
                }
            }
        } else if (buf[0] == 'O') {
            switch(buf[1]) {
                case 'H': return HOME;
                case 'F': return END;
            }
        }
        return '\x1b';
    }
    return c;
}

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
            break;
        case K_CTRL('q'): {
            if (config.modified && quitStroke > 0) {
                Alert::setStatusMsg(this->config, "UNSAVED CHANGES | Press quit again to discard changes");
                quitStroke--;
                break;
            }
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
            // handle delete
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

void Editor::insertCharAtRow( char c) {
    size_t insertRowNum = this->config.cy;
    size_t insertColNum = this->config.cx;

    std::shared_ptr<Row> row = this->config.fileData.at(insertRowNum);

    // Allow insertion at end of row
    if (insertColNum > row->sRaw.size()) {
        insertColNum = row->sRaw.size();
    }

    // TODO: this isn't great... we probably shouldn't need to update sRender as well
    // Insert a single character
    row->sRaw.insert(insertColNum, 1, c);
    row->sRender = row->sRaw;
    config.modified++;
}

void Editor::insertChar(char c) {
    // TODO: Make an append function? Needs to also update modified
    if (this->config.cy == this->config.fileData.size()) {
        config.fileData.emplace_back(std::make_shared<Row>(""));
    }
    insertCharAtRow(c);
    this->config.cx++;
}

