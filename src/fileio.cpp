#include <fileio.hh>
#include <errmgr.hh>
#include <fstream>
#include <regex>
#include <sstream>
#include <config.hh>
#include <iostream>

int FileIO::openFile(TTEdFileData& fileData, const char *path) {
    std::ifstream ifs(path);
    if (!ifs) ErrorMgr::err("failed to open file");
    fileData.filename = path;

    std::string spaceStr(TABSTOP, ' ');

    std::string s1;
    while (std::getline(ifs, s1)) {
        std::string s2 = std::regex_replace(s1, std::regex("\t"), spaceStr);
        fileData.fileData.emplace_back(std::make_shared<Row>(Row{s1, s2}));
        // fileData.insertRow(fileData.size, Row{s1, s2});
        fileData.size++;
    }

    ifs.close();
    fileData.modified = 0; // TODO: remove and place after error checking when implemented
    return ifs.failbit;
}

int FileIO::saveFile(TTEdFileData& fileData) {
    std::stringstream ss = fileData.streamify();
    std::ofstream ofs(fileData.filename, std::ofstream::trunc);
    ofs << ss.str();
    
    ofs.close();
    fileData.modified = 0; // TODO: remove and place after error checking when implemented
    return ofs.failbit;
}
