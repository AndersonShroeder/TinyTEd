#include <fileio.hh>
#include <errmgr.hh>
#include <fstream>
#include <regex>

void FileIO::openFile(Config& config, const char *path) {
    std::ifstream file(path);
    if (!file) ErrorMgr::err("failed to open file");
    config.filename = path;

    std::string spaceStr(TABSTOP, ' ');

    std::string s1;
    while (std::getline(file, s1)) {
        std::string s2 = std::regex_replace(s1, std::regex("\t"), spaceStr);
        config.fileData.emplace_back(std::make_shared<Row>(Row{s1, s2}));
    }

    file.close();
}