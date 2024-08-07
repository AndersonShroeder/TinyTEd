#pragma once

#include <string>
#include <config.hh>
#include <editor.hh>
#include <sstream>

/**
 * @class TerminalGUI
 * @brief Manages the visual aspects of the terminal interface.
 */
class TerminalGUI {
private:
    /**
     * @brief Buffer for accumulating terminal commands.
     */
    std::stringstream buf;

    /**
     * @brief Reference to the configuration object holding editor state.
     */
    Config& config;

    /**
     * @brief Clears the screen.
     */
    void wipeScreen();

    /**
     * @brief Resets the cursor position to the top-left corner of the screen.
     */
    void resetCursor();

    /**
     * @brief Hides the cursor to prevent flickering.
     */
    void hideCursor();

    /**
     * @brief Shows the cursor.
     */
    void showCursor();

    /**
     * @brief Converts the cursor position to the render position for the row.
     * @param row The row to render to.
     */
    int rowCxToRx(std::shared_ptr<Row> row);

    /**
     * @brief Handles scrolling of the text and cursor.
     */
    void scroll();

    /**
     * @brief Flushes the accumulated terminal commands from the buffer to the terminal.
     */
    void flushBuf();

    /**
     * @brief Updates the cursor position on the screen.
     */
    void updateCursor();

    /**
     * @brief Draws the rows of text on the screen.
     */
    void drawRows();

    /**
     * @brief Draws the status bar at the bottom of the screen.
     */
    void drawStatusBar();

    /**
     * @brief Draws the message bar at the bottom of the screen.
     */
    void drawMessageBar();

    /**
     * @brief Centers the given text within the current screen width.
     * 
     * @param text The text to be centered.
     * @return The centered text.
     */
    static std::string centerText(Config& config, std::string s);

    /**
     * @brief Generates the cover page text for the splash screen.
     * 
     * @param config The configuration object.
     * @param s The string to store the generated cover page text.
     */
    static void genCoverPage(Config& config, std::string &s);

public:
    /**
     * @brief Constructs a TerminalGUI instance.
     * 
     * @param cfg The configuration object to initialize the terminal GUI.
     */
    TerminalGUI(Config& cfg);

    /**
     * @brief Draws the entire terminal interface including text, status bar, and message bar.
     */
    void draw();

    /**
     * @brief Displays the splash screen and waits for user input.
     * 
     * @param e The editor instance used to process user input.
     */
    void splashScreen(Editor &e);

    /**
     * @brief Gets the current cursor position in the terminal.
     * 
     * @param config The configuration object to store the cursor position.
     * @return 0 on success, -1 on failure.
     */
    static int getCursorPosition(Config& config);

    /**
     * @brief Initializes the terminal GUI, setting up initial screen size and other settings.
     */
    void initGUI();

    /**
     * @brief Resets the screen.
     */
    void reset();
};
