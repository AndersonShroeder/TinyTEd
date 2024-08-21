#pragma once

#include <string>
#include <vector>

struct syntaxHL {
    std::string filetype;
    std::string comment;
    std::vector<std::string> keywords;
    std::vector<std::string> types;
    std::vector<std::string> extensions;
};

static const std::vector<syntaxHL> hlSchemes = {

};