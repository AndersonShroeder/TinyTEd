#include <config.hh>
#include <sstream>
#include <unistd.h>
#include <sys/ioctl.h>

///////////////////
// ROW METHODS
///////////////////

void Row::insertChar(TTEdCursor &cursor, char c) {
    // Allow insertion at end of row
    size_t insertColNum = cursor.cx > this->size ? this->size : cursor.cx;

    // Insert a single character
    this->sRaw.insert(insertColNum, 1, c);
    this->sRender = this->sRaw;
    this->size++;
}

void Row::deleteChar(TTEdCursor &cursor) {
    // Delete char left of curosr
    size_t delColNum =cursor.cx - 1;

    this->sRaw.erase(delColNum, 1);
    this->sRender = this->sRaw;
    this->size--;
}

Row Row::splitRow(TTEdCursor &cursor) {
    // Make new row
    std::string sub = this->sRaw.substr(cursor.cx, this->size);
    Row newRow{sub, sub};

    // Update curr row
    this->sRaw = this->sRaw.substr(0, cursor.cx);
    this->sRender = this->sRaw;
    this->size = sRaw.size();

    return newRow;
}

///////////////////
// TERMDATA METHODS
///////////////////

void TTEdTermData::getWindowSize() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // ErrorMgr::err("unable to retrieve window size");
        //TODO: ERROR OUT IDK
    } else {
        this->sCol = ws.ws_col;
        this->sRow = ws.ws_row - 2;
    }
}

void TTEdTermData::enterRaw() {
    if (tcgetattr(STDIN_FILENO, &(this->tty)) == -1) {
    //    TODO: Error out
    }
    
    struct termios raw = this->tty;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    //    TODO: Error out
    }
}

void TTEdTermData::exitRaw() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &(this->tty)) == -1) {
        // TODO: ERROR out
    }
}

///////////////////
// FILEDATA METHODS
///////////////////

void TTEdFileData::insertRow(size_t pos, Row row) {
    this->fileData.insert(this->fileData.begin() + pos, std::make_shared<Row>(row));
}

void TTEdFileData::insertChar(TTEdCursor &cursor, char c) {
    // If inserting at EOF make a new line to insert onto
    if (cursor.cy == this->size) {
        this->insertRow(cursor.cy);
    }
    
    std::shared_ptr<Row> insertRow = this->fileData.at(cursor.cy);
    insertRow->insertChar(cursor, c);
    cursor.cx++;
    this->modified++;
}

void TTEdFileData::deleteChar(TTEdCursor &cursor) {
    
}

void TTEdFileData::insertNewLine(TTEdCursor &cursor) {
    // Split row only if not at start of line
    if (cursor.cx == 0) {
        this->insertRow(cursor.cy);
    } else {
        std::shared_ptr<Row> rowToSplit = this->fileData.at(cursor.cy);
        Row newRow = rowToSplit->splitRow(cursor);
        this->insertRow(cursor.cy, newRow);
    }

    // Want cursor to "follow" the insertions
    cursor.cy++;
    cursor.cx = 0;
}

std::stringstream TTEdFileData::streamify() {
    std::stringstream ss;
    for (std::shared_ptr<Row> r: this->fileData) {
        ss << r->sRaw << '\n';
    }
    return ss;
}