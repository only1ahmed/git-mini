#include "../gitmini.h"
#include "../gitminiHelper.h"
#include <fstream>
#include <iostream>


namespace fs = std::filesystem;

// recursive function to process each directory (and its subdirectories) and map hashes and snapshots to the new commit.
// function return the number of files/folder in the directory and the hash of the treeObject after saving it.
std::pair<std::string, int>
processDirectory(std::string oldCommitCurrentTreeHash, fs::path currentDirectory,
                 std::unordered_map<fs::path, gitminiHelper::stageObject> &stagedChanges) {

    int numFilesInDirectory = 0;
    std::unordered_map<fs::path, gitminiHelper::treeFile> newCommitDirectoryHashes;
    if (not oldCommitCurrentTreeHash.empty()) {
        newCommitDirectoryHashes = gitminiHelper::readTreeObject(
                oldCommitCurrentTreeHash);
        numFilesInDirectory = gitminiHelper::countFilesTreeHash(oldCommitCurrentTreeHash);
    }
    std::string treeHash;

    std::unordered_map<std::string, std::vector<gitminiHelper::stageObject>> groups;

    for (auto change: stagedChanges) {
        std::string groupDir = change.first.begin()->string();
        change.second.path = change.first;
        gitminiHelper::stageObject &file = change.second;
        int pathSize = std::distance(change.first.begin(), change.first.end());
        // this is a file!
        if (pathSize == 1) {
            if (not(file.type == gitminiHelper::stageObject::TYPE::FILE or
                    file.type == gitminiHelper::stageObject::TYPE::FOLDER)) {
                //TODO: raise and error here. where there is only one element left in the path but that element is not a file.
            }
            //NOTE: there SHALL NOT be any folders that gets created or modified because folders cannot be modified and are only created once there are files inside of them (so creating a file inside the folder will create the folder automatically in the tree).
            if (file.operation == gitminiHelper::stageObject::OPERATION::MODIFY ||
                file.operation == gitminiHelper::stageObject::OPERATION::CREATE) {
                newCommitDirectoryHashes[file.path].type = gitminiHelper::objectType::BLOB;
                newCommitDirectoryHashes[file.path].name = file.path.string();
                newCommitDirectoryHashes[file.path].hash = file.hash;
                if (file.operation == gitminiHelper::stageObject::OPERATION::CREATE) {
                    numFilesInDirectory++;
                }
            }
            // However, folders can be removed! as all files exist only because the folder exists in the parent's tree children.
            if (file.operation == gitminiHelper::stageObject::OPERATION::DELETE) {
                newCommitDirectoryHashes.erase(file.path);
                numFilesInDirectory--;
            }
            continue;
        }
        groups[groupDir].push_back(change.second);
    }
    for (auto &group: groups) {
        std::string groupDir = group.first;
        std::unordered_map<fs::path, gitminiHelper::stageObject> tempMap;
        for (auto &file: group.second) {
            fs::path newPath;
            bool con = true;
            // trimming the first element of the path.
            for (auto &name: file.path) {
                if (con) {
                    con = false;
                    continue;
                }
                newPath /= name;
            }

            gitminiHelper::stageObject newFile = file;
            newFile.path = newPath;
            tempMap[newPath] = newFile;
        }
        // if num is 0, erase;
        std::string subdirectoryHash;
        if (not oldCommitCurrentTreeHash.empty()) {
            subdirectoryHash = gitminiHelper::findFileHash(currentDirectory / groupDir, oldCommitCurrentTreeHash);
        }
        auto result = processDirectory(subdirectoryHash, currentDirectory / groupDir, tempMap);
        if (result.second == 0) {
            newCommitDirectoryHashes.erase(currentDirectory / groupDir);
            continue;
        }
        newCommitDirectoryHashes[currentDirectory / groupDir].type = gitminiHelper::objectType::TREE;
        newCommitDirectoryHashes[currentDirectory / groupDir].name = groupDir;
        newCommitDirectoryHashes[currentDirectory / groupDir].hash = result.first;
        numFilesInDirectory += result.second;

    }
    std::string treeContent = gitminiHelper::structureTreeObject(newCommitDirectoryHashes);
    treeHash = gitminiHelper::hashFile(treeContent, gitminiHelper::objectHeader(
            std::to_string(gitminiHelper::objectType::TREE),
            treeContent.size()));
    gitminiHelper::saveObject(treeHash, treeContent, gitminiHelper::objectHeader(
            std::to_string(gitminiHelper::objectType::TREE),
            treeContent.size()));

    return std::make_pair(treeHash, numFilesInDirectory);
}


void navigate() {

}

// TODO: implement args navigation
void gitmini::commit(const std::vector<std::string> &args) {

//    navigate()

    // load staged and current commit.
    gitminiHelper::loadCurrentCommit(this->currentCommitHash, gitmini::branchTracer);
    gitminiHelper::loadStagedChanges(this->stagedChanges, gitmini::stageTracer);

    std::string &parentCommitHash = this->currentCommitHash;
    gitminiHelper::commitObject parentCommitData = gitminiHelper::readCommitHash(parentCommitHash);
    // get the root tree from processing the stages.
    std::string newCommitRoot = processDirectory(parentCommitData.root, "", this->stagedChanges).first;

    std::string commitContent = gitminiHelper::structureCommit({{"parent",  parentCommitHash},
                                                                {"root",    newCommitRoot},
                                                                {"message", ""}});

    std::string commitHash = gitminiHelper::hashFile(commitContent,
                                                     gitminiHelper::objectHeader(
                                                             std::to_string(gitminiHelper::objectType::COMMIT),
                                                             commitContent.size()));
    gitminiHelper::saveObject<std::string>(commitHash, commitContent, gitminiHelper::objectHeader(
            std::to_string(gitminiHelper::objectType::COMMIT),
            commitContent.size()));
    std::ofstream writeBranchTracerFile(gitmini::branchTracer, std::ios::out);
    writeBranchTracerFile << commitHash;
    writeBranchTracerFile.close();
}

