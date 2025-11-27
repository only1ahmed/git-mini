#include "../gitmini.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;


void navigate() {

}

//TODO: apply gitmini diff to know whether a file had changes or not before staging it.
void gitmini::add(const std::vector<fs::path> &args) {
    if (args.empty()) {
        std::cerr << "Error: No files were provided" << std::endl;
        return;
    }
    if (*args.begin() == "*") {

    }
    std::ofstream outFile(gitmini::stageTracer);

    for (const auto &filePath: args) {
        if ((this->stagedFiles.count(filePath) < 1) && (this->ignoredFiles.count(filePath) < 1) &&
            fs::exists(filePath)) {
            outFile << filePath.string() + '\n';
            this->stagedFiles.insert(filePath);
        }
    }
    outFile.flush();
}