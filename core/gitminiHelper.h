#pragma once

#include <unordered_set>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
namespace gitminiHelper {

//    template<typename Container>
//    void static loadFiles(Container &, const fs::path &);

    void loadFiles(std::unordered_set<std::string> &files, const fs::path &fileDirectory);
}