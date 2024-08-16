#pragma once

#include <config.hh>
#include <termgui.hh>

/**
 * @namespace Commands
 * @brief Provides functionality for commands within the text editor.
 */
namespace Commands
{

    /**
     * @namespace Search
     * @brief Implements the search command
     */
    namespace Search
    {
        /**
         * @brief Callback function to handle search results or actions.
         *
         * @param cfg The configuration object containing editor state and settings.
         * @param s The search query string.
         * @param c The current match index or other relevant integer data.
         */
        void callback(Config &cfg, std::string_view s, int c);

        /**
         * @brief Runs the search command, triggering the search process and updating the GUI.
         *
         * @param gui The TerminalGUI object used for displaying search results.
         * @param cfg The configuration object containing editor state and settings.
         */
        void run(TerminalGUI &gui, Config &cfg);
    }
};
