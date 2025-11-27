#pragma once

#include <unordered_set>
#include <string>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;
namespace gitminiHelper {

    template<typename T>
    concept StringOrPath = std::is_same_v<T, std::string> || std::is_same_v<T, fs::path>;

    void loadFiles(std::unordered_set<fs::path> &files, const fs::path &fileDirectory);

    std::string processDirectoryTree(const fs::path &root);

    template<StringOrPath T>
    std::string hashFile(const T &content, std::string header);

    template<StringOrPath T>
    void saveObject(const std::string &hash, const T &content, const std::string &header);

    std::string structureCommit(const std::map<std::string, std::string> &);

    std::string objectHeader(const std::string &type, int contentSize);
}