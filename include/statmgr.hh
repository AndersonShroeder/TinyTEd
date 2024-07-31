#pragma once

#include <config.hh>

/**
 * @class StateMgr
 * @brief Manages the terminal's state transitions for raw input mode.
 */
class StateMgr {
public:
    /**
     * @brief Enters raw mode for terminal input processing.
     * 
     * @param config The configuration object holding terminal settings.
     */
    static void enterRaw(Config& config);

    /**
     * @brief Exits raw mode and restores the previous terminal settings.
     * 
     * @param config The configuration object holding terminal settings.
     */
    static void exitRaw(Config& config);
};