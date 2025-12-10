#include "../gitmini.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

//TODO: make sure the paths that come to "add" are valid paths (exists and withing the repo folder) and that the repo exists.


// this should return the paths that we are going to process.
std::vector<fs::path> navigate() {
//    if (*args.begin() == ".") {
//
//    }
}

//TODO: apply gitmini diff to know whether a file had changes or not before staging it.
void gitmini::add(const std::vector<fs::path> &args) {

//    navigate();

    if (args.empty()) {
        std::cerr << "Error: No files were provided" << std::endl;
        return;
    }
    // load staged and ignored files and current commit.
    gitminiHelper::loadCurrentCommit(this->commitRoot, gitmini::branchTracer);
    gitminiHelper::loadStagedChanges(this->stagedChanges, gitmini::stageTracer);
    gitminiHelper::loadIgnored(this->ignoredFiles, gitmini::ignoredFilesPath);

    // all these paths shall be files.
    for (const auto &filePath: args) {
        if ((this->ignoredFiles.count(filePath) > 0)) {
            continue;
        }
        if (not fs::exists(filePath)) {
            //TODO:
            // throw an error
            continue;
        }

        //hash the file.
        std::string snapshotHash = gitminiHelper::hashFile(filePath,
                                                           gitminiHelper::objectHeader(
                                                                   std::to_string(gitminiHelper::objectType::BLOB),
                                                                   fs::file_size(filePath)));
        //traverse the commit tree to get the file hash, then compare it to the snapshot hash to detect whether there are changes or not.
        std::string fileHash = gitminiHelper::findFileHash(filePath, this->commitRoot);
        if (fileHash == snapshotHash) {
            continue;
        }
        //you have to delete the old snapshot to avoid overloading the memory.
        //make sure not to create the file before deleting the old one.
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
        gitminiHelper::saveObject(snapshotHash, filePath,
                                  gitminiHelper::objectHeader(std::to_string(gitminiHelper::objectType::BLOB),
                                                              fs::file_size(filePath)));
        stagedChanges[filePath].type = gitminiHelper::stageObject::TYPE::FILE;
        if (fileHash != "") {

            stagedChanges[filePath].operation = gitminiHelper::stageObject::OPERATION::MODIFY;
        } else {
            stagedChanges[filePath].operation = gitminiHelper::stageObject::OPERATION::CREATE;
        }
        stagedChanges[filePath].hash = snapshotHash;

//        }

        //save the stage tracer.
    }
    gitminiHelper::saveStagedChanges(this->stagedChanges, gitmini::stageTracer);
}