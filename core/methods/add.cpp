#include "../gitmini.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;


void navigate() {

}

void gitmini::add(const std::vector<std::string> &args) {
    if (args.empty()) {
        std::cerr << "Error: No files were provided" << std::endl;
        return;
    }
    if (*args.begin() == "*") {

    }
    std::ofstream outFile(this->stagedFilePath);

    for (const auto &filename: args) {
        if ((this->stagedFiles.count(filename) < 1) && (this->ignoredFiles.count(filename) < 1)) {
            outFile << filename + '\n';
            this->stagedFiles.insert(filename);
        }
    }
}