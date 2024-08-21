#pragma once

#include <string>
#include <vector>

struct SyntaxHL {
    std::string filetype;
    std::string comment;
    std::vector<std::string> keywords;
    std::vector<std::string> types;
    std::vector<std::string> extensions;
};

static const std::vector<SyntaxHL> hlSchemes = {
  {
    "C++",
    "//",
    {},
    {},
    {"C", "cc", "cpp", "CPP", "c++", "cp", "cxx"}
  }
};
