#pragma once

#include <unordered_map>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <unordered_set>

namespace fs = std::filesystem;
namespace gitminiHelper {
    enum objectType {
        BLOB,
        TREE,
        COMMIT,
        PATH

    };
    struct stageObject {
        enum OPERATION {
            MODIFY,
            CREATE,
            DELETE
        };
        enum TYPE {
            FILE,
            FOLDER
        };
        fs::path path;
        std::string hash;
        OPERATION operation;
        TYPE type;
    };

    struct treeFile {

        std::string name, hash;
        objectType type;
    };

    struct commitObject {
        // parent hash
        std::string branch;
        std::string parent;
        // parent hash
        std::string root;
        std::string message;

    };

//    std::string FILESHEADER = "blob ";

    template<typename T>
    concept StringOrPath = std::is_same_v<T, std::string> || std::is_same_v<T, fs::path>;

    std::string processDirectoryTree(const fs::path &root);

    template<StringOrPath T>
    std::string hashFile(const T &content, std::string header);


    template<StringOrPath T>
    void saveObject(const std::string &hash, const T &content, const std::string &header,
                    std::ios_base::openmode writingMode = std::ios::trunc);

    std::string readObject(const std::string &hash);

    std::string structureCommit(const std::map<std::string, std::string> &);

    std::string objectHeader(const std::string &type, int contentSize);

    void
    loadStagedChanges(std::unordered_map<fs::path, gitminiHelper::stageObject> &files, const fs::path &fileDirectory);

    void
    saveStagedChanges(std::unordered_map<fs::path, gitminiHelper::stageObject> &files, const fs::path &fileDirectory);

    std::string findFileHash(const fs::path &, const std::string &);

    void loadIgnored(std::unordered_set<fs::path> &, const fs::path &);

    void loadCurrentCommit(std::string &, const fs::path &);

    std::string hashToPath(std::string);

    std::unordered_map<fs::path, gitminiHelper::treeFile> readTreeObject(std::string &hash);

    std::string structureTreeObject(std::unordered_map<fs::path, gitminiHelper::treeFile> &treeFiles);

    int countFilesTreeHash(std::string &hash);

    gitminiHelper::commitObject readCommitHash(std::string &hash);

    fs::path findInParentPath(const fs::path &start, const fs::path &target);
}