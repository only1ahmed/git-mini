#pragma once

#include <string>
#include "gitminiHelper.h"
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class gitmini {

public:
    inline static const fs::path baseFolderPath = ".gitmini";
    inline static const fs::path ignoredFilesPath = ".gitminiignore";
    inline static const fs::path objectsFolderPath = baseFolderPath / "objects";
    inline static const fs::path localRefsFolderPath = baseFolderPath / "refs";
    inline static const fs::path localHeadsFolderPath = localRefsFolderPath / "heads";
    inline static const fs::path infoFolder = baseFolderPath / "info";
    inline static const fs::path stageTracer = infoFolder / "staged";
    inline static const fs::path branchTracer = infoFolder / "current";

    std::unordered_map<fs::path, std::vector<std::string>> stagedChanges;
    std::unordered_set<fs::path> ignoredFiles;
    std::string commitRoot;

    gitmini();

    void init();

    void add(const std::vector<fs::path> &);

    void rm(const std::vector<fs::path> &);

    void commit(const std::vector<std::string> &);


};
