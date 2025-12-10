//TODO: create the .gitmini directory
//TODO: add stage buffer logic
//NOTE: the buffer is independent from the branching logic
#include "../gitmini.h"
#include <filesystem>
#include <fstream>
#include <iostream>


namespace fs = std::filesystem;
const std::string MAINBRANCHNAME = "main";


void gitmini::init() {

// check whether there exists a current git repo in the directory.
    fs::path folderPath = fs::current_path() / this->baseFolderPath;
    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        std::cout << "There is an already existing repo." << std::endl;
        //TODO: add an option to override the current (or never mind lol).
        return;
    }
    //note: std::ios::out is used to ensure overwriting over all opened files.
    try {
        fs::create_directory(gitmini::baseFolderPath);
        fs::create_directories(gitmini::objectsFolderPath);
        fs::create_directories(gitmini::localRefsFolderPath);
        fs::create_directories(gitmini::localHeadsFolderPath);
        fs::create_directories(gitmini::infoFolder);
        std::ofstream(this->stageTracer, std::ios::out);
        std::ofstream branchTracerFile(gitmini::branchTracer, std::ios::out);
        std::string initCommitContent = gitminiHelper::structureCommit({{"parent",  ""},
                                                                        {"root",    ""},
                                                                        {"message", ""}});
        std::string initCommitHash = gitminiHelper::hashFile(initCommitContent,
                                                             gitminiHelper::objectHeader(
                                                                     std::to_string(gitminiHelper::objectType::COMMIT),
                                                                     initCommitContent.size()));
        gitminiHelper::saveObject(initCommitHash, initCommitContent, gitminiHelper::objectHeader(
                std::to_string(gitminiHelper::objectType::COMMIT),
                initCommitContent.size()));
        branchTracerFile << initCommitHash;
        branchTracerFile.close();
        std::ofstream mainBranchFile(gitmini::localHeadsFolderPath / MAINBRANCHNAME, std::ios::out);
        mainBranchFile << initCommitHash;
        mainBranchFile.close();


    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }


}