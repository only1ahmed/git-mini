#include "gitmini.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;


void gitmini::loadStagedFiles() {

    if (not fs::exists(this->stagedFilePath)) {
        std::ofstream outfile(this->stagedFilePath);
        if (outfile) {
            outfile.close();
        } else {
            std::cerr << "Error: Failed to create " << this->stagedFilePath.string() << std::endl;
        }

    }
    std::ifstream infile(this->stagedFilePath);
    if (!infile) {
        std::cerr << "Error: Failed to open " << this->stagedFilePath << std::endl;
        return;
    }
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.empty()) {
            this->stagedFiles.insert(line);
        }
    }
}

void gitmini::loadBaseFolder() {
    if (not fs::exists(this->baseFolderPath)) {
        if (not fs::create_directory(this->baseFolderPath)) {
            std::cerr << "Error: Failed to create " << this->stagedFilePath.string() << std::endl;
        }
    }

}


gitmini::gitmini() {
    // Load staged files.
    this->loadBaseFolder();
    gitminiHelper::loadFiles(this->stagedFiles, this->stagedFilePath);
    gitminiHelper::loadFiles(this->ignoredFiles, this->ignoredFilesPath);

//    this->loadStagedFiles();
//    this->loadIgnoredFiles();

}