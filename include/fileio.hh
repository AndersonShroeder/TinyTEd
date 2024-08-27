#pragma once

#include <config.hh>
#include <string>

void parseFileExtension(Config &cfg);

/**
 * @class FileIO
 * @brief Provides functionality for file input/output operations.
 */
class FileIO
{
public:
    /**
     * @brief Opens a file and loads its contents into the editor's configuration.
     *
     * @param config The configuration object where file data will be loaded.
     * @param path The path to the file to be opened.
     * @return 0 on success, -1 on failure.
     */
    static int openFile(Config &cfg, const std::string &path);

    /**
     * @brief Saves the current editor content to a file.
     *
     * @param cfg The configuration object containing the file data to be saved.
     * @return 0 on success, -1 on failure.
     */
    static int saveFile(Config &cfg);
};
