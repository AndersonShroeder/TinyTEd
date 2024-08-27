#pragma once

#include <config.hh>
#include <termgui.hh>
#include <optional>
#include <functional>

/**
 * @namespace InputHandler
 * @brief Provides functionality for handling user input, cursor movement, and interactions.
 */
namespace InputHandler
{
    // Convenience enum for checking return values of processKey
    enum procval
    {
        FAILURE = -1,   ///< Indicates a failure in processing the key.
        SUCCESS = 0,    ///< Indicates successful processing of the key.
        PROMPTSAVE = 1, ///< Indicates a prompt to save the current file.
        PROMPTSEARCH,   ///< Indicates a prompt to start a search.
        PROMPTSERVER,
        PROMPTCONNECT,
        SHUTDOWN,       ///< Indicates a request to shut down the editor.
    };

    /**
     * @brief Moves the cursor based on the given command.
     *
     * @param cursor The cursor object that will be moved.
     * @param fData The file data, which may affect cursor movement.
     * @param c The command for cursor movement (e.g., ARROW_UP, ARROW_DOWN).
     */
    void moveCursor(TTEdCursor &cursor, TTEdFileData &fData, int c);

    /**
     * @brief Prompts the user with a message and optionally executes a command based on the response.
     *
     * @param gui The TerminalGUI object used for displaying prompts.
     * @param cfg The configuration object containing editor state.
     * @param msg The message to display to the user.
     * @param cmd An optional command to execute based on the user response.
     * @return The user input as a string.
     */
    std::string promptUser(TerminalGUI &gui, Config &cfg, const std::string &msg, std::optional<TTEdCommand> cmd);

    /**
     * @brief Processes a key input and performs the associated action.
     *
     * @param cursor The cursor object affected by the key input.
     * @param fData The file data that may be modified by the key input.
     * @param term The terminal data for processing the key input.
     * @param breakAny If true, will break out of the function immediately upon processing the key.
     * @return A value from the procval enum indicating the result of the key processing.
     */
    int processKey(TTEdCursor &cursor, TTEdFileData &fData, TTEdTermData &term, TTEdStatus &stat, bool breakAny = false);
};
