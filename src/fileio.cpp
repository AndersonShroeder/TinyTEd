#include <fileio.hh>
#include <errmgr.hh>
#include <fstream>
#include <regex>
#include <sstream>
#include <config.hh>

int FileIO::openFile(Config& config, const char *path) {
    std::ifstream ifs(path);
    if (!ifs) ErrorMgr::err("failed to open file");
    config.filename = path;

    std::string spaceStr(TABSTOP, ' ');

    std::string s1;
    while (std::getline(ifs, s1)) {
        std::string s2 = std::regex_replace(s1, std::regex("\t"), spaceStr);
        config.fileData.emplace_back(std::make_shared<Row>(Row{s1, s2}));
    }

    ifs.close();
    config.modified = 0; // TODO: remove and place after error checking when implemented
    return ifs.failbit;
}

int FileIO::saveFile(Config& cfg) {
    std::stringstream ss = ConfigTools::accFileData(cfg);
    std::ofstream ofs(cfg.filename, std::ofstream::trunc);
    ofs << ss.str();
    
    ofs.close();
    cfg.modified = 0; // TODO: remove and place after error checking when implemented
    return ofs.failbit;
}
