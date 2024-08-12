#pragma once

/**
 * @class ErrorMgr
 * @brief Provides error handling functionality for the application.
 */
class ErrorMgr
{
public:
    /**
     * @brief Handles and reports an error message.
     *
     * @param s The error message to be reported.
     */
    static void err(const char *s);
};
