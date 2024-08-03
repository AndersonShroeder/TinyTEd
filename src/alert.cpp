#include <alert.hh>

void Alert::setStatusMsg(Config &cfg, const std::string &msg) {
    cfg.statusMsg = msg;
    cfg.statusTime = std::time(nullptr);
}
