#include <inhandler.hh>
#include <errmgr.hh>
#include <termgui.hh>
#include <fileio.hh>
#include <unistd.h>
#include <inreader.hh>

void InputHandler::moveCursor(TTEdCursor &cursor, TTEdFileData &fData, int c) {
    std::shared_ptr<Row> data = cursor.cy >= fData.size() ? nullptr : fData.at(cursor.cy);
    
    switch(c) {
        case ARROW_UP:
            if (cursor.cy > 0) cursor.cy--;
            break;
        case ARROW_LEFT:
            if (cursor.cx != 0) {
                cursor.cx--;
            } else if (cursor.cy > 0) {
                cursor.cy--;
                cursor.cx = fData.at(cursor.cy)->sRaw.size();
            }
            break;
        case ARROW_DOWN:
            if (cursor.cy < fData.size()) cursor.cy++;
            break;
        case ARROW_RIGHT:
            if (data && cursor.cx < data->sRaw.size()) {
                cursor.cx++;
            } else if (data && cursor.cx == data->sRaw.size()) {
                cursor.cy++;
                cursor.cx = 0;
            }
            break;
    }

    data = cursor.cy >= fData.size() ? nullptr : fData.at(cursor.cy);
    size_t newlen = data ? data->sRaw.size() : 0;
    if (cursor.cx > newlen) {
        cursor.cx = newlen;
    }
}

int InputHandler::processKey(TTEdCursor &cursor, TTEdFileData &fData, TTEdTermData &term, bool breakAny) {
    static int quitStroke = 1;
    int c = InputReader::readKey();
    if (breakAny) return 0;
    switch (c) {
        // TODO: Newline
        case '\r':
            fData.insertNewLine(cursor);
            break;
        case K_CTRL('q'): {
            // if (modified && quitStroke > 0) {
            //     Alert::setStatusMsg(this->config, "UNSAVED CHANGES | Press quit again to discard changes");
            //     quitStroke--;
            //     break;
            // }
            return -1;
        }

        case K_CTRL('s'):
            if (1 == FileIO::saveFile(fData)) {
                // TODO: error Checking
            }
            break;

        case PAGE_UP:
        case PAGE_DOWN: {
            if (c == PAGE_UP) {
                cursor.cy = cursor.rOffset;
            } else if (c == PAGE_DOWN) {
                cursor.cy = cursor.rOffset + term.sRow - 1;
                if (cursor.cy > fData.size()) {
                    cursor.cy = fData.size();
                }
            }
            
            int times = term.sRow;
            while (times--) {
                moveCursor(cursor, fData, c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        }

        case HOME:
            cursor.cx = 0;
            break;
        case END:
            if (cursor.cy < fData.size()) {
                cursor.cx = fData.at(cursor.cy)->sRaw.size();
            }
            break;

        // TODO: implement
        case BACKSPACE:
        case K_CTRL('h'):
        case DEL:
            if (c == DEL) moveCursor(cursor, fData, ARROW_RIGHT); // delete character to the right of cursor currently
            fData.deleteChar(cursor);
            break;

        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            moveCursor(cursor, fData, c);
            break;

        // Ignore for now
        case K_CTRL('l'):
        case '\x1b':
            break;

        default:
            // handle other keys
            fData.insertChar(cursor, c);
            break;
    }

    quitStroke = 1;
    return 0;
}