#include "../gitmini.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

void gitmini::rm(const std::vector<fs::path> &args) {
    if (args.empty()) {
        std::cerr << "Error: No files were provided" << std::endl;
        return;
    }
    gitminiHelper::loadCurrentCommit(this->commitRoot, gitmini::branchTracer);
    gitminiHelper::loadStagedChanges(this->stagedChanges, gitmini::stageTracer);
    gitminiHelper::loadIgnored(this->ignoredFiles, gitmini::ignoredFilesPath);

    for (const auto &filePath: args) {
        if ((this->ignoredFiles.count(filePath) > 0)) {
            continue;
        }
        if (not fs::exists(filePath)) {
            //TODO:
            // throw an error
            continue;
        }
        
        //you have to delete the old snapshot to avoid overloading the memory.
        try {
            fs::path oldSnapshotPath;
            if (stagedChanges.count(filePath) > 0) {
                oldSnapshotPath = gitmini::objectsFolderPath / gitminiHelper::hashToPath(stagedChanges[filePath].hash);
            }
            if (fs::exists(oldSnapshotPath)) {
                std::filesystem::remove(oldSnapshotPath);
            }
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "Error deleting file: " << e.what() << "\n";
        }

        stagedChanges[filePath].type = gitminiHelper::stageObject::TYPE::FILE;
        stagedChanges[filePath].operation = gitminiHelper::stageObject::OPERATION::DELETE;


        //save the stage tracer.
        gitminiHelper::saveStagedChanges(this->stagedChanges, gitmini::stageTracer);
    }
}