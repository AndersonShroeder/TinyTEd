#include <config.hh>
#include <sstream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctime>

///////////////////
// ROW METHODS
///////////////////

void Row::insertChar(TTEdCursor &cursor, char c) {
    // Insert character at the cursor position or end of the row
    size_t insertColNum = cursor.cx > this->size() ? this->size() : cursor.cx;

    this->sRaw.insert(insertColNum, 1, c);
    this->sRender = this->sRaw; // Update rendered string
}

void Row::deleteChar(TTEdCursor &cursor) {
    // Delete character to the left of the cursor
    size_t delColNum = cursor.cx > 0 ? cursor.cx - 1 : 0;

    this->sRaw.erase(delColNum, 1);
    this->sRender = this->sRaw; // Update rendered string
}

size_t Row::size() const {
    return this->sRaw.size(); // Return the length of the raw string
}

Row Row::splitRow(TTEdCursor &cursor) {
    // Split the row at the cursor position and create a new row with the split part
    std::string sub = this->sRaw.substr(cursor.cx);
    Row newRow{sub, sub};

    this->sRaw = this->sRaw.substr(0, cursor.cx);
    this->sRender = this->sRaw; // Update rendered string

    return newRow;
}

///////////////////
// CURSOR METHODS
///////////////////

int TTEdCursor::rowCxToRx(std::shared_ptr<Row> row) {
    rx = 0; // Reset rendered x position
    for (size_t j = 0; j < this->cx; j++) {
        if (row->sRaw.at(j) == '\t') {
            // Adjust for tab stops
            rx += (TABSTOP - 1) - (rx % TABSTOP);
        }
        rx++;
    }
    return rx;
}

///////////////////
// STATUS METHODS
///////////////////

void TTEdStatus::setStatusMsg(const std::string &msg) {
    this->statusMsg = msg;
    this->statusTime = std::time(nullptr); // Update the timestamp
}

void TTEdStatus::resetStatusMsg() {
    this->statusMsg.clear(); // Clear the status message
    this->statusTime = std::time(nullptr); // Update the timestamp
}

///////////////////
// TERMDATA METHODS
///////////////////

void TTEdTermData::getWindowSize() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // TODO: Handle error for unable to retrieve window size
    } else {
        this->sCol = ws.ws_col;
        this->sRow = ws.ws_row - 2; // Adjust for status bar
    }
}

void TTEdTermData::enterRaw() {
    if (tcgetattr(STDIN_FILENO, &(this->tty)) == -1) {
        // TODO: Handle error for unable to get terminal attributes
    }

    struct termios raw = this->tty;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        // TODO: Handle error for unable to set terminal attributes
    }
}

void TTEdTermData::exitRaw() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &(this->tty)) == -1) {
        // TODO: Handle error for unable to reset terminal attributes
    }
}

///////////////////
// FILEDATA METHODS
///////////////////

void TTEdFileData::insertRow(size_t pos, Row row) {
    // Insert a new row at the specified position
    this->fileData.insert(this->fileData.begin() + pos, std::make_shared<Row>(row));
}

size_t TTEdFileData::size() const {
    return this->fileData.size(); // Return the number of rows
}

std::shared_ptr<Row> TTEdFileData::at(size_t pos) const {
    return this->fileData.at(pos); // Access row at specified position
}

void TTEdFileData::insertChar(TTEdCursor &cursor, char c) {
    // Insert a character into the file data at the cursor position
    if (cursor.cy == this->size()) {
        this->insertRow(cursor.cy); // Add a new row if at end of file
    }
    
    std::shared_ptr<Row> insertRow = this->fileData.at(cursor.cy);
    insertRow->insertChar(cursor, c);
    cursor.cx++;
    this->modified++;
}

void TTEdFileData::deleteChar(TTEdCursor &cursor) {
    if (cursor.cy >= this->size() || (cursor.cx == 0 && cursor.cy == 0)) {
        return; // No action if at the start or invalid position
    }

    if (cursor.cx > 0) {
        this->at(cursor.cy)->deleteChar(cursor);
        cursor.cx--;
    } 
    else {
        // Merge current row with the previous row
        size_t oldcy = cursor.cy;
        size_t newcy = --(cursor.cy);

        // Append the current row to the previous row
        cursor.cx = this->at(newcy)->sRaw.size();
        this->at(newcy)->sRaw += this->at(oldcy)->sRaw;
        this->at(newcy)->sRender += this->at(oldcy)->sRender;

        // Remove the old row
        this->fileData.erase(this->fileData.begin() + oldcy);
    }

    this->modified++;
}

void TTEdFileData::insertNewLine(TTEdCursor &cursor) {
    // Insert a new line at the cursor position
    if (cursor.cx == 0) {
        this->insertRow(cursor.cy);
    } else {
        std::shared_ptr<Row> rowToSplit = this->fileData.at(cursor.cy);
        Row newRow = rowToSplit->splitRow(cursor);
        this->insertRow(cursor.cy + 1, newRow);
    }

    // Move cursor to the new line
    cursor.cy++;
    cursor.cx = 0;
}

std::stringstream TTEdFileData::streamify() {
    std::stringstream ss;
    for (const auto& r : this->fileData) {
        ss << r->sRaw << '\n'; // Write each row to the stringstream
    }
    return ss;
}

void Config::scroll() {
    cursor.rx = 0; // Reset horizontal cursor position

    // Adjust row offset
    if (cursor.cy < fileData.size()) {
        cursor.rx = cursor.rowCxToRx(fileData.at(cursor.cy));
    }

    if (cursor.cy < cursor.rOffset) {
        cursor.rOffset = cursor.cy;
    }

    if (cursor.cy >= cursor.rOffset + term.sRow) {
        cursor.rOffset = cursor.cy - term.sRow + 1;
    }

    // Adjust column offset
    if (cursor.rx < cursor.cOffset) {
        cursor.cOffset = cursor.rx;
    }

    if (cursor.rx >= cursor.cOffset + term.sCol) {
        cursor.cOffset = cursor.rx - term.sCol + 1;
    }
}
