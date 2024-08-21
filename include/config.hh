#pragma once

#include <string>
#include <vector>
#include <memory>
#include <termios.h>
#include <functional>

#define TABSTOP 4
#define K_CTRL(k) ((k) & 0x1f)

enum keys
{
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

enum textState
{
    TS_NORMAL = 0,
    TS_NUMBER,
    TS_SEARCH
};

struct TTEdCursor;
struct Row;
struct TTEdStatus;
struct TTEdTermData;
struct TTEdFileData;
struct Config;

using TTEdCommand = std::function<void(Config &, std::string, int)>;

/**
 * @struct TTEdCursor
 * @brief Represents the cursor's position and offset in the editor.
 *
 * @param rOffset Row offset for vertical scrolling.
 * @param cOffset Column offset for horizontal scrolling.
 * @param cx Cursor's x position on the screen.
 * @param cy Cursor's y position on the screen.
 * @param rx Rendered x position in the file row.
 */
struct TTEdCursor
{
    size_t rOffset = 0;
    size_t cOffset = 0;
    size_t cx = 0;
    size_t cy = 0;
    size_t rx = 0;

    /**
     * @brief Converts the cursor position to the render position for the row.
     *
     * @param row A shared pointer to the row to render to.
     * @return The rendered x position.
     */
    int rowCxToRx(std::shared_ptr<Row> row);
};

/**
 * @struct Row
 * @brief Represents a single row of text in the editor.
 *
 * @param sRaw Raw representation of file row data.
 * @param sRender Parsed representation accounting for tabs.
 * @param textState Render state for ith element
 */
struct Row
{
    std::string sRaw;
    std::string sRender;
    std::array<textState, UCHAR_MAX> textStates;

    /**
     * @brief Inserts a character at the current cursor position.
     *
     * @param cursor A reference to the cursor object.
     * @param c The character to insert.
     */
    void insertChar(TTEdCursor &cursor, char c);

    /**
     * @brief Deletes a character at the current cursor position.
     *
     * @param cursor A reference to the cursor object.
     */
    void deleteChar(TTEdCursor &cursor);

    /**
     * @brief Gets the size of the row.
     *
     * @return The number of characters in the row.
     */
    size_t size() const;

    /**
     * @brief Splits the row at the current cursor position.
     *
     * @param cursor A reference to the cursor object.
     * @return A new Row object representing the split part.
     */
    Row splitRow(TTEdCursor &cursor);

    /**
     * @brief Parses the state of each character in the row
    */
    void parseStates();


    void updateRender();

    bool isSeparator(int c);
};

/**
 * @struct TTEdStatus
 * @brief Represents the status message and timestamp for the editor.
 *
 * @param statusMsg The current status message.
 * @param statusTime The time when the status message was set.
 */
struct TTEdStatus
{
    std::string statusMsg;
    time_t statusTime = 0;

    /**
     * @brief Sets the status message.
     *
     * @param msg The new status message.
     */
    void setStatusMsg(const std::string &msg);

    /**
     * @brief Resets the status message to an empty state.
     */
    void resetStatusMsg();
};

/**
 * @struct TTEdTermData
 * @brief Contains terminal-related data and operations.
 *
 * @param sRow Number of rows in the terminal window.
 * @param sCol Number of columns in the terminal window.
 * @param tty Terminal I/O settings.
 */
struct TTEdTermData
{
    size_t sRow;
    size_t sCol;
    struct termios tty;

    /**
     * @brief Gets the current size of the terminal window.
     *
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

/**
 * @struct TTEdFileData
 * @brief Represents the file data currently open in the editor.
 *
 * @param filename The name of the file.
 * @param fileData Vector of shared pointers to Row objects representing file content.
 * @param modified Flag indicating whether the file has been modified.
 */
struct TTEdFileData
{
    std::string filename;
    std::vector<std::shared_ptr<Row>> fileData;
    int modified = 0;

    /**
     * @brief Inserts a row at the specified position.
     *
     * @param pos The position to insert the row.
     * @param row The row to insert (default is an empty row).
     */
    void insertRow(size_t pos, Row row = {"", ""});

    /**
     * @brief Gets the number of rows in the file.
     *
     * @return The number of rows.
     */
    size_t size() const;

    /**
     * @brief Gets a shared pointer to a row at the specified position.
     *
     * @param pos The position of the row.
     * @return A shared pointer to the row.
     */
    std::shared_ptr<Row> at(size_t pos) const;

    /**
     * @brief Inserts a character at the current cursor position.
     *
     * @param cursor A reference to the cursor object.
     * @param c The character to insert.
     */
    void insertChar(TTEdCursor &cursor, char c);

    /**
     * @brief Deletes a character at the current cursor position.
     *
     * @param cursor A reference to the cursor object.
     */
    void deleteChar(TTEdCursor &cursor);

    /**
     * @brief Inserts a new line at the current cursor position.
     *
     * @param cursor A reference to the cursor object.
     */
    void insertNewLine(TTEdCursor &cursor);

    /**
     * @brief Converts the file data to a string stream representation.
     *
     * @return A string stream containing the file data.
     */
    std::stringstream streamify();
};

/**
 * @struct Config
 * @brief Configuration and state information for the editor.
 *
 * @param cursor The current cursor state.
 * @param term Terminal-related data and settings.
 * @param fileData The data of the currently open file.
 * @param status The current status message and timestamp.
 */
struct Config
{
    TTEdCursor cursor;
    TTEdTermData term;
    TTEdFileData fileData;
    TTEdStatus status;

    /**
     * @brief Handles scrolling of the text and cursor.
     */
    void scroll();
};
