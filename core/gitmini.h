#pragma once

#include <string>
#include "gitminiHelper.h"
#include <unordered_set>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class gitmini {

private:
    fs::path baseFolderPath = ".gitmini";
    fs::path stagedFilePath = baseFolderPath / "staged";
    fs::path ignoredFilesPath = ".gitminiignore";
    std::unordered_set<std::string> stagedFiles;
    std::unordered_set<std::string> ignoredFiles;


    //TODO
    // Note: you need to figure out somehow to get the current directory from the CLI.
public:
    gitmini();

//    gitmini(std::string);

    void loadStagedFiles();


    void loadBaseFolder();

    void init();

    void add(const std::vector<std::string> &);

};
