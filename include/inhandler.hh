#pragma once

#include <config.hh>
#include <termgui.hh>

/**
 * @class Editor
 * @brief Represents the text editor, managing user input and cursor movement.
 */
namespace InputHandler {
    // Convience enum for checking return of processKey
    enum procval {
        FAILURE = -1,
        SUCCESS = 0,
        PROMPTSAVE = 1,
        SHUTDOWN = 2,
    };

    /**
     * @brief Moves the cursor based on the given command.
     * 
     * @param c The command for cursor movement (e.g., ARROW_UP, ARROW_DOWN).
     */
    void moveCursor(TTEdCursor &cursor, TTEdFileData &fData, int c);

    std::string promptUser(TerminalGUI &gui, TTEdStatus &stat, std::string msg);

    /**
     * @brief Processes a key input and performs the associated action.
     * 
     * @param breakAny If true, will break out of the function immediately.
     */
    int processKey(TTEdCursor &cursor, TTEdFileData &fData, TTEdTermData &term, bool breakAny = false);
};
