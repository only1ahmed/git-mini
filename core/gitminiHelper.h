#pragma once

#include <unordered_map>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <unordered_set>

namespace fs = std::filesystem;
namespace gitminiHelper {
    class stageObject {
    public:
        enum OPERATION {
            ALTER,
            DELETE
        };
        enum TYPE {
            FILE,
            FOLDER
        };
        std::string hash;
        OPERATION operation;
        TYPE type;
    };
//    std::string FILESHEADER = "blob ";

    template<typename T>
    concept StringOrPath = std::is_same_v<T, std::string> || std::is_same_v<T, fs::path>;

    std::string processDirectoryTree(const fs::path &root);

    template<StringOrPath T>
    std::string hashFile(const T &content, std::string header);

    template<StringOrPath T>
    void saveObject(const std::string &hash, const T &content, const std::string &header);

    std::string readObject(const std::string &);

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


}