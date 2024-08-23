#include <fileio.hh>
#include <errmgr.hh>
#include <fstream>
#include <sstream>
#include <config.hh>
#include <iostream>
#include <filesystem>

void parseFileExtension(Config &cfg) {
  cfg.syntax = NULL;

  cfg.fileData.filename = cfg.fileData.path.filename();
  cfg.fileData.extension = cfg.fileData.path.extension();
  
  for (auto &syntax: hlSchemes) {
    auto extensions = syntax.extensions;
    if (std::find(extensions.begin(), extensions.end(), cfg.fileData.extension) != extensions.end()) {
      cfg.syntax = &syntax;

      
      // Rehighlight files based on new syntax
      for (auto &r: cfg.fileData.fileData) {
        r->updateRender();
      }

      return;
    }
  }
}

int FileIO::openFile(Config &cfg, const std::string &path)
{
    std::ifstream ifs(path);
    if (!ifs)
    {
        ErrorMgr::err("Failed to open file"); // Report the file path in error
        return -1;                            // Return an error code
    }
    std::filesystem::path p(path);
    cfg.fileData.path = path;

    parseFileExtension(cfg);

    std::string line;
    while (std::getline(ifs, line))
    {
        auto r = std::make_shared<Row>(Row{line});
        r->updateRender();

        cfg.fileData.fileData.emplace_back(r);
    }

    ifs.close();
    cfg.fileData.modified = 0;       // Reset modified flag after successful file load
    return ifs.failbit ? -1 : 0; // Return error code if failbit is set
}

int FileIO::saveFile(Config &cfg)
{
    std::stringstream ss = cfg.fileData.streamify();
    std::ofstream ofs(cfg.fileData.path, std::ofstream::trunc);

    if (!ofs)
    {
        ErrorMgr::err("Failed to open file for writing"); // Report the file path in error
        return -1;                                        // Return an error code
    }

    ofs << ss.str();
    ofs.close();
    cfg.fileData.modified = 0;       // Reset modified flag after successful save
    
    parseFileExtension(cfg);

    return ofs.failbit ? -1 : 0; // Return error code if failbit is set
}
