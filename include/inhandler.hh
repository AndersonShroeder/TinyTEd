#pragma once

#include <config.hh>

/**
 * @class Editor
 * @brief Represents the text editor, managing user input and cursor movement.
 */
namespace InputHandler {
    /**
     * @brief Moves the cursor based on the given command.
     * 
     * @param c The command for cursor movement (e.g., ARROW_UP, ARROW_DOWN).
     */
    void moveCursor(TTEdCursor &cursor, TTEdFileData &fData, int c);

    /**
     * @brief Processes a key input and performs the associated action.
     * 
     * @param breakAny If true, will break out of the function immediately.
     */
    int processKey(TTEdCursor &cursor, TTEdFileData &fData, TTEdTermData &term, bool breakAny = false);
};
