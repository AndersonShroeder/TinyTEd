#pragma once

#include <string>
#include <config.hh>
#include <sstream>

/**
 * @class TerminalGUI
 * @brief Manages the visual aspects of the terminal interface.
 */
class TerminalGUI
{
private:
    /**
     * @brief Buffer for accumulating terminal commands.
     */
    std::stringstream buf;

    /**
     * @brief Reference to the configuration object holding editor state.
     */
    Config &config;

    /**
     * @brief Flushes the accumulated terminal commands from the buffer to the terminal.
     */
    void flushBuf();

    /**
     * @brief Updates the cursor position on the screen.
     *
     * @param cursor The cursor object containing the current cursor position.
     */
    void updateCursor(const TTEdCursor &cursor);

    /**
     * @brief Draws the rows of text on the screen.
     *
     * @param cursor The cursor object containing the current cursor position.
     * @param fData The file data containing the text to be displayed.
     * @param tData The terminal data containing display parameters.
     */
    void drawRows(const TTEdCursor &cursor, const TTEdFileData &fData, const TTEdTermData &tData);

    /**
     * @brief Draws the status bar at the bottom of the screen.
     *
     * @param cursor The cursor object containing the current cursor position.
     * @param fData The file data containing the text to be displayed.
     * @param tData The terminal data containing display parameters.
     */
    void drawStatusBar(const TTEdCursor &cursor, const TTEdFileData &fData, const TTEdTermData &tData);

    /**
     * @brief Draws the message bar at the bottom of the screen.
     *
     * @param tData The terminal data containing display parameters.
     * @param status The status object containing the message to be displayed.
     */
    void drawMessageBar(const TTEdTermData &tData, const TTEdStatus &status);

    /**
     * @brief Centers the given text within the current screen width.
     *
     * @param config The configuration object.
     * @param text The text to be centered.
     * @return The centered text.
     */
    static std::string centerText(const Config &config, const std::string &text);

    /**
     * @brief Generates the cover page text for the splash screen.
     *
     * @param config The configuration object.
     * @param s The string to store the generated cover page text.
     */
    static void genCoverPage(const Config &config, std::string &s);

public:
    /**
     * @brief Constructs a TerminalGUI instance.
     *
     * @param cfg The configuration object to initialize the terminal GUI.
     */
    TerminalGUI(Config &cfg);

    /**
     * @brief Draws the entire terminal interface including text, status bar, and message bar.
     */
    void draw();

    /**
     * @brief Displays the splash screen and waits for user input.
     */
    void splashScreen();

    /**
     * @brief Resets the screen.
     */
    void reset();
};
