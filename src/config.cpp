#include <config.hh>
#include <sstream>

std::stringstream ConfigTools::accFileData(Config &cfg) {
    std::stringstream ss;
    for (std::shared_ptr<Row> r: cfg.fileData) {
        ss << r->sRaw << '\n';
    }
    return ss;
}