#pragma once

#include <sstream>

namespace TermActions {
    /**
     * @brief Clears the screen.
     */
    void wipeScreen(std::stringstream &buf);

    /**
     * @brief Resets the cursor position to the top-left corner of the screen.
     */
    void resetCursor(std::stringstream &buf);

    /**
     * @brief Hides the cursor to prevent flickering.
     */
    void hideCursor(std::stringstream &buf);

    /**
     * @brief Shows the cursor.
     */
    void showCursor(std::stringstream &buf);
};