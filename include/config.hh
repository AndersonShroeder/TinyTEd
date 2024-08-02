#pragma once
#include <string>
#include <vector>
#include <memory>
#include <termios.h>

#define TABSTOP 4

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
    size_t sRow;
    size_t sCol;
    size_t rOffset;
    size_t cOffset;
    size_t cx;
    size_t cy;
    size_t rx;
    int modified = 0;
    struct termios tty;
    std::vector<std::shared_ptr<Row>> fileData;
    std::string filename;
    std::string statusMsg;
    time_t statusTime = 0;
};

namespace ConfigTools {
    std::stringstream accFileData(Config &cfg);
}