#pragma once

#include <string>
#include <vector>
#include <memory>
#include <termios.h>
#include <functional>

#define TABSTOP 4
#define K_CTRL(k) ((k) & 0x1f)

enum keys {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
};

struct TTEdCursor;
struct Row;
struct TTEdStatus;
struct TTEdTermData;
struct TTEdFileData;
struct Config;

using TTEdCommand = std::function<void(Config &, std::string, int)>;

struct TTEdCursor {
    size_t rOffset = 0;
    size_t cOffset= 0;
    size_t cx = 0;
    size_t cy = 0;
    size_t rx = 0;

    // Methods
    /**
     * @brief Converts the cursor position to the render position for the row.
     * @param row The row to render to.
     */
    int rowCxToRx(std::shared_ptr<Row> row);
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

    void insertChar(TTEdCursor &cursor, char c);
    void deleteChar(TTEdCursor &cursor);
    size_t size();
    Row splitRow(TTEdCursor &cursor);
};

struct TTEdStatus {
    std::string statusMsg;
    time_t statusTime = 0;

    // Methods
    void setStatusMsg(const std::string &msg);
    void resetStatusMsg();
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
    int modified = 0;

    // Methods
    void insertRow(size_t pos, Row row = {"", ""});
    size_t size();
    std::shared_ptr<Row> at(size_t pos);

    // These three functions could maybe be moved into the main config struct
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

    /**
     * @brief Handles scrolling of the text and cursor.
     */
    void scroll();
};