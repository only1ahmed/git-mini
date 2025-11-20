//TODO: create the .gitmini directory
//TODO: add stage buffer logic
//NOTE: the buffer is independent from the branching logic
#include "../gitmini.h"
#include <filesystem>

#include <iostream>


namespace fs = std::filesystem;

void gitmini::init() {

// check whether there exists a current git repo in the directory.
    fs::path folderPath = fs::current_path() / this->baseFolderPath;
    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        std::cout << "There is an already existing repo." << std::endl;
        return;
    }
    fs::create_directory(this->baseFolderPath);


}