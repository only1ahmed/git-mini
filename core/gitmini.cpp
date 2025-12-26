#include "gitmini.h"
#include <filesystem>
#include <fstream>
#include <iostream>

//TODO: EXCEPTION HANDLING!!!!

namespace fs = std::filesystem;


gitmini::gitmini() {
    try {
        fs::path currentDirectory = gitminiHelper::findInParentPath(
                fs::current_path(),
                static_cast<fs::path>(baseFolderPath)
        );

        if (currentDirectory.empty()) {
            throw fs::filesystem_error(
                    "gitmini wasn't found", baseFolderPath, std::error_code()
            );
        }

        fs::current_path(currentDirectory);
    }
    catch (const fs::filesystem_error &e) {
        std::cerr << "Error in constructor: " << e.what() << std::endl;
    }
}
