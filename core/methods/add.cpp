#include "../gitmini.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

//TODO: make sure the paths that come to "add" are valid paths (exists and withing the repo folder) and that the repo exists.

namespace addCommands {
    std::vector<fs::path> all_entries(std::unordered_set<fs::path> &ignoredFiles) {
        std::vector<fs::path> result;
        for (auto it = fs::recursive_directory_iterator(fs::current_path());
             it != fs::recursive_directory_iterator(); ++it) {
            const fs::directory_entry &entry = *it;

            if (entry.path() == (fs::current_path() / gitmini::baseFolderPath)) {
                it.disable_recursion_pending();
                continue;
            }
            if (entry.is_regular_file() and (not ignoredFiles.contains(entry.path()))) {
                result.push_back(entry.path());
            }
        }
        return result;
    }
}


// this should return the paths that we are going to process.
std::vector<fs::path> navigate(const std::vector<std::string> &args, std::unordered_set<fs::path> &ignoredFiles) {
    try {
        std::vector<fs::path> result;
        std::string argsBegin = *args.begin();
        if (argsBegin == ".") {
            result = addCommands::all_entries(ignoredFiles);
        } else {
            for (auto &path: args) {
                result.emplace_back(static_cast<fs::path> (path));
            }
        }

        return result;
    }
    catch (std::filesystem::filesystem_error &e) {
        throw e;
    }

}

// TODO: implement args navigation

void gitmini::add(const std::vector<std::string> &args) {
    std::vector<fs::path> files;
    try {
        if (args.empty()) {
            std::cerr << "Error: No args were provided" << std::endl;
            return;
        }
        files = navigate(args, this->ignoredFiles);


        // load staged and ignored files and current commit.
        gitminiHelper::loadCurrentCommit(this->currentCommitHash, gitmini::branchTracer);
        std::string currentCommitRoot = gitminiHelper::readCommitHash(this->currentCommitHash).root;
        gitminiHelper::loadStagedChanges(this->stagedChanges, gitmini::stageTracer);
        gitminiHelper::loadIgnored(this->ignoredFiles, gitmini::ignoredFilesPath);

        // all these paths shall be files.
        for (const auto &file: files) {
            fs::path filePath = fs::relative(file, fs::current_path());
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
            std::string fileHash = gitminiHelper::findFileHash(filePath, currentCommitRoot);
            if (fileHash == snapshotHash) {
                continue;
            }
            //you have to delete the old snapshot to avoid overloading the memory.
            //make sure not to create the file before deleting the old one.

            fs::path oldSnapshotPath;
            if (stagedChanges.count(filePath) > 0) {
                oldSnapshotPath =
                        gitmini::objectsFolderPath / gitminiHelper::hashToPath(stagedChanges[filePath].hash);
            }
            if (fs::exists(oldSnapshotPath)) {
                std::filesystem::remove(oldSnapshotPath);
            }

            gitminiHelper::saveObject(snapshotHash, filePath,
                                      gitminiHelper::objectHeader(std::to_string(gitminiHelper::objectType::BLOB),
                                                                  fs::file_size(filePath)));
            stagedChanges[filePath].type = gitminiHelper::stageObject::TYPE::FILE;
            if (not fileHash.empty()) {

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
    catch (std::filesystem::filesystem_error &e) {
        std::cerr << e.what() << std::endl;
    }
}