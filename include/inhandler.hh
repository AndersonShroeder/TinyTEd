#pragma once

#include <config.hh>
#include <termgui.hh>
#include <optional>
#include <functional>

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
        PROMPTSEARCH,
        SHUTDOWN,
    };

    /**
     * @brief Moves the cursor based on the given command.
     * 
     * @param c The command for cursor movement (e.g., ARROW_UP, ARROW_DOWN).
     */
    void moveCursor(TTEdCursor &cursor, TTEdFileData &fData, int c);

    std::string promptUser(TerminalGUI &gui, Config &cfg, std::string msg, std::optional<TTEdCommand> cmd);

    /**
     * @brief Processes a key input and performs the associated action.
     * 
     * @param breakAny If true, will break out of the function immediately.
     */
    int processKey(TTEdCursor &cursor, TTEdFileData &fData, TTEdTermData &term, bool breakAny = false);
};
