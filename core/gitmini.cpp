#include "gitmini.h"
#include <filesystem>
#include <fstream>
#include <iostream>

//TODO: EXCEPTION HANDLING!!!!

namespace fs = std::filesystem;


void gitmini::loadBaseFolder() {
    if (not fs::exists(this->baseFolderPath)) {
        std::cerr << "Error: Didn't find a repository in the current directory."
                  << std::endl;
    }

}

//TODO: load tree folder
// tree folder will contain hashes of branch names as folders.
// each folder will contain the parents and children information (names).
// same logic with the content tree.
// hash paths -> create folders of these hashes -> store in the folder the children and parents paths.
// create a folder "objects" that will contain the files contents (perhaps compressed).
// in objects, hash each file's (branch + path)
gitmini::gitmini() {
    // Load staged files.
//    this->loadBaseFolder();
    if (fs::exists(gitmini::baseFolderPath)) {
        gitminiHelper::loadFiles(this->stagedFiles, gitmini::stageTracer);
        gitminiHelper::loadFiles(this->ignoredFiles, gitmini::ignoredFilesPath);
    }

//    this->loadStagedFiles();
//    this->loadIgnoredFiles();

}