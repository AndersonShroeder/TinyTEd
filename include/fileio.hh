#pragma once

#include <config.hh>

/**
 * @class FileIO
 * @brief Provides functionality for file input/output operations.
 */
class FileIO {
public:
    /**
     * @brief Opens a file and loads its contents into the editor's configuration.
     * 
     * @param config The configuration object where file data will be loaded.
     * @param path The path to the file to be opened.
     */
    static int openFile(Config& config, const char *path);
    static int saveFile(Config& cfg);
};