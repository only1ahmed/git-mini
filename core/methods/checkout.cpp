#include "../gitmini.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>

namespace fs = std::filesystem;
const bool DEBUG = true;
namespace checkoutCommands {

    void clear_directory(const fs::path &dir) {
        for (const auto &entry: fs::directory_iterator(dir)) {
            if (DEBUG) {
                if (entry.path() == (fs::current_path() / "test.exe")) {
                    continue;
                }
            }
            if (entry.path() == (fs::current_path() / gitmini::baseFolderPath)) {
                continue;
            }
            fs::remove_all(entry);

        }
    }
}


void commitProcessing(std::string &commitHash) {
    gitminiHelper::commitObject commitData = gitminiHelper::readCommitHash(commitHash);

    std::unordered_map<fs::path, gitminiHelper::treeFile> commitTreeData = gitminiHelper::readTreeObject(
            commitData.root);
    std::queue<std::unordered_map<fs::path, gitminiHelper::treeFile>> q;
    q.push(commitTreeData);
    while (not q.empty()) {
        commitTreeData = q.front();
        q.pop();
        for (auto &entry: commitTreeData) {
            fs::path path = entry.first;
            gitminiHelper::treeFile file = entry.second;
            if (file.type == gitminiHelper::objectType::BLOB) {
                std::string blobContent = gitminiHelper::readObject(file.hash);
                std::ofstream newFile(path);
                newFile << blobContent;
                newFile.close();
            }
            if (file.type == gitminiHelper::objectType::TREE) {
                if (not fs::exists(path)) {
                    fs::create_directory(path);
                }
                q.push(gitminiHelper::readTreeObject(file.hash));
            }

        }
    }
}


//TODO: navigation
//TODO: handle "if you checkout to a previous commit on the same branch, then try to commit a new one" case. (apply branching to avoid losing commits)
void gitmini::checkout(const std::vector<std::string> &args) {
    // navigate
    //TODO: implement navigate to return the commit hash
    std::string commitHash = args[0];

    checkoutCommands::clear_directory(fs::current_path());

    commitProcessing(commitHash);

    std::ofstream writeBranchTracerFile(gitmini::branchTracer, std::ios::out);
    writeBranchTracerFile << commitHash;
    writeBranchTracerFile.close();

}