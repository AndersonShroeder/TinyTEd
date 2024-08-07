#pragma once
#include <string>
#include <vector>
#include <memory>
#include <termios.h>

#define TABSTOP 4

struct TTEdCursor {
    size_t rOffset;
    size_t cOffset;
    size_t cx;
    size_t cy;
    size_t rx;

    // Methods
};

/**
 * @struct Row
 * @brief Represents a single row of text in the editor.
 * 
 * @param sRaw Raw representation of file row data.
 * @param sRender Parsed representation accounting for tabs.
 */
struct Row {
    std::string sRaw;
    std::string sRender;
    size_t size;

    void insertChar(TTEdCursor &cursor, char c);
    void deleteChar(TTEdCursor &cursor);
    Row splitRow(TTEdCursor &cursor);
};

struct TTEdStatus {
    std::string statusMsg;
    time_t statusTime = 0;

    // Methods

};

struct TTEdTermData {
    size_t sRow;
    size_t sCol;
    struct termios tty;

    /**
     * @brief Gets the current size of the terminal window.
     * 
     * @param config The configuration object to store the window size.
     * @return 0 on success, -1 on failure.
     */
    void getWindowSize();

    /**
     * @brief Enters raw mode for terminal input processing.
     * 
     * @param config The configuration object holding terminal settings.
     */
    void enterRaw();

    /**
     * @brief Exits raw mode and restores the previous terminal settings.
     * 
     * @param config The configuration object holding terminal settings.
     */
    void exitRaw();
};


struct TTEdFileData {
    std::string filename;
    std::vector<std::shared_ptr<Row>> fileData;
    size_t size;
    int modified = 0;

    // Methods
    void insertRow(size_t pos, Row row = {"", ""});
    void insertChar(TTEdCursor &cursor, char c);
    void deleteChar(TTEdCursor &cursor);
    void insertNewLine(TTEdCursor &cursor);
    std::stringstream streamify();
};

/**
 * @struct Config
 * @brief Configuration and state information for the editor.
 * 
 * @param sRow Screen height in rows.
 * @param sCol Screen width in columns.
 * @param rOffset Row offset for vertical scrolling.
 * @param cOffset Column offset for horizontal scrolling.
 * @param cx Cursor's x position on the screen.
 * @param cy Cursor's y position on the screen.
 * @param rx Rendered x position in the file row.
 * @param modified Whether or not the file opened has been modified
 * @param tty Terminal I/O settings.
 * @param fileData Data of the file currently open, stored as rows.
 * @param filename Name of the currently open file.
 * @param statusMsg Current status message.
 * @param statusTime Time when the status message was set.
 */
struct Config {
    TTEdCursor cursor;
    TTEdTermData term;
    TTEdFileData fileData;
    TTEdStatus status;
};