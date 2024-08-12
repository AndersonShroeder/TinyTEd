#include <fileio.hh>
#include <errmgr.hh>
#include <fstream>
#include <regex>
#include <sstream>
#include <config.hh>
#include <iostream>

int FileIO::openFile(TTEdFileData &fileData, const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs)
    {
        ErrorMgr::err("Failed to open file"); // Report the file path in error
        return -1;                            // Return an error code
    }

    fileData.filename = path;
    std::string spaceStr(TABSTOP, ' '); // String of spaces to replace tabs

    std::string line;
    while (std::getline(ifs, line))
    {
        // Replace tabs with spaces and store both raw and rendered versions of the line
        std::string renderedLine = std::regex_replace(line, std::regex("\t"), spaceStr);
        fileData.fileData.emplace_back(std::make_shared<Row>(Row{line, renderedLine}));
    }

    ifs.close();
    fileData.modified = 0;       // Reset modified flag after successful file load
    return ifs.failbit ? -1 : 0; // Return error code if failbit is set
}

int FileIO::saveFile(TTEdFileData &fileData)
{
    std::stringstream ss = fileData.streamify();
    std::ofstream ofs(fileData.filename, std::ofstream::trunc);

    if (!ofs)
    {
        ErrorMgr::err("Failed to open file for writing"); // Report the file path in error
        return -1;                                        // Return an error code
    }

    ofs << ss.str();
    ofs.close();
    fileData.modified = 0;       // Reset modified flag after successful save
    return ofs.failbit ? -1 : 0; // Return error code if failbit is set
}
