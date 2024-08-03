#pragma once

#include <config.hh>

/**
 * @class Editor
 * @brief Represents the text editor, managing user input and cursor movement.
 */
class Editor {
private:
    /**
     * @brief Reference to the configuration object holding editor state.
     */
    Config& config;

    /**
     * @brief File descriptor for reading input.
     */
    int r;

    /**
     * @brief Reads a single key from input.
     * 
     * @return The ASCII value of the key read.
     */
    int readKey();

    /**
     * @brief Moves the cursor based on the given command.
     * 
     * @param c The command for cursor movement (e.g., ARROW_UP, ARROW_DOWN).
     */
    void moveCursor(int c);

public:
    /**
     * @brief Constructs an Editor instance.
     * 
     * @param cfg The configuration object to initialize the editor state.
     */
    Editor(Config& cfg);

    /**
     * @brief Processes a key input and performs the associated action.
     * 
     * @param breakAny If true, will break out of the function immediately.
     */
    int processKey(bool breakAny = false);

    void insertCharAtRow(char c);

    void insertChar(char c);

    void delCharAtRow();
    
    void delChar();

    void insertNewLine();
};
