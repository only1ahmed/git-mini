#include "gitminiHelper.h"
#include <fstream>
#include <iostream>

void gitminiHelper::loadFiles(std::unordered_set<std::string> &files, const fs::path &fileDirectory) {

    if (not fs::exists(fileDirectory)) {
        std::ofstream outfile(fileDirectory);
        if (outfile) {
            outfile.close();
        } else {
            std::cerr << "Error: Found, but Failed to open and check " << fileDirectory << std::endl;
            return;
        }

    }
    std::ifstream infile(fileDirectory);
    if (!infile) {
        std::cerr << "Error: Found, Failed to open and write on " << fileDirectory << std::endl;
        return;
    }
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.empty()) {
            files.insert(line);
        }
    }
}
