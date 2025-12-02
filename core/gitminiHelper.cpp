#include "gitminiHelper.h"
#include "gitmini.h"
#include <fstream>
#include <iostream>
#include <map>
#include <openssl/sha.h>
#include <vector>
// to ignore the SHA256 deprecation
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include <sstream>

const size_t BUFFER_SIZE = (1 << 16); //64 kb
const std::vector<std::string> COMMITELEMENTS = {"parent", "root", "message"};


void gitminiHelper::loadCurrentCommit(std::string &refCurrentCommit, const fs::path &filePath) {
    std::ifstream file(filePath);
    file >> refCurrentCommit;
}

void gitminiHelper::loadIgnored(std::unordered_set<fs::path> &ignoredFiles, const fs::path &filePath) {
    std::ifstream file(filePath);
    std::string path;
    while (file >> path) {
        ignoredFiles.insert(fs::path(path));
    }

}


void
gitminiHelper::loadStagedChanges(std::unordered_map<fs::path, gitminiHelper::stageObject> &files,
                                 const fs::path &fileDirectory) {

    if (not fs::exists(fileDirectory)) {
        std::cerr << "Error: Didn't find: " << fileDirectory.string() << std::endl;
        return;

    }
    std::ifstream infile(fileDirectory);
    if (!infile) {
        std::cerr << "Error: Found, Failed to open and write on " << fileDirectory.string() << std::endl;
        return;
    }
    std::stringstream ss;
    ss << infile.rdbuf();
    int type, operation;
    std::string hash;
    std::string path;
    while (ss >> path) {
        if (path.empty()) {
            break;
        }
        ss >> type >> operation;
        files[path].type = static_cast<gitminiHelper::stageObject::TYPE>(type);
        files[path].operation = static_cast<gitminiHelper::stageObject::OPERATION>(operation);
        if (type == gitminiHelper::stageObject::TYPE::FILE &&
            operation == gitminiHelper::stageObject::OPERATION::ALTER) {
            ss >> hash;
            files[path].hash = hash;
        }

    }
}

void gitminiHelper::saveStagedChanges(std::unordered_map<fs::path, gitminiHelper::stageObject> &files,
                                      const fs::path &fileDirectory) {

    std::ofstream outFile(fileDirectory);
    if (!outFile) {
        std::cerr << "Error: Failed to create or write on " << fileDirectory.string() << std::endl;
        return;
    }

    int type, operation;
    std::string hash;
    std::string path;
    std::string result;
    for (auto file: files) {
        path = file.first.string();
        type = file.second.type;
        operation = file.second.operation;
        result += path + ' ' + std::to_string(type) + ' ' + std::to_string(operation) + ' ';
        if (type == gitminiHelper::stageObject::TYPE::FILE &&
            operation == gitminiHelper::stageObject::OPERATION::ALTER) {
            hash = file.second.hash;
            result += hash;
        }
        result += '\n';
    }
    outFile << result;
}

template<gitminiHelper::StringOrPath T>
std::string gitminiHelper::hashFile(const T &content, std::string header) {

    std::vector<unsigned char> buffer(BUFFER_SIZE);
    try {

        std::unique_ptr<std::istream> file;

        if constexpr (std::is_same_v<T, std::string>) {
            file = std::make_unique<std::stringstream>(content, std::ios::binary);
        } else if constexpr (std::is_same_v<T, fs::path>) {
            auto f = std::make_unique<std::ifstream>(content, std::ios::binary);
            if (!*f) {
                throw std::runtime_error("Failed to open file: " + content.string());
            }
            file = std::move(f); // transfer ownership to polymorphic pointer
        }


        // Enable exceptions for stream errors
        file->exceptions(std::ifstream::badbit);

        SHA256_CTX ctx;
        SHA256_Init(&ctx);

        SHA256_Update(&ctx, header.data(), header.size());

        while (true) {
            file->read(reinterpret_cast<char *>(buffer.data()), buffer.size());
            //gcount() returns the number of chars that had been extracted in the last OPERATION
            std::streamsize bytesRead = file->gcount();
            if (bytesRead > 0) {
                SHA256_Update(&ctx, buffer.data(), bytesRead);
            }
            if (file->eof()) {
                break;
            }
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &ctx);

        std::ostringstream result;
        for (unsigned char i: hash) {
            result << std::hex << std::setw(2) << std::setfill('0') << (int) i;
        }

        return result.str();
    }
    catch (const std::exception &e) {
        std::cerr << "Error while hashing file: " << e.what() << "\n";
        return "";
    }
}


template<gitminiHelper::StringOrPath T>
void gitminiHelper::saveObject(const std::string &hash, const T &content, const std::string &header) {

    fs::path hashPath = gitminiHelper::hashToPath(hash);
    fs::path filePath = gitmini::objectsFolderPath / hashPath;

    if (fs::exists(filePath)) {
        return;
    }

    std::unique_ptr<std::istream> file;

    if constexpr (std::is_same_v<T, std::string>) {
        file = std::make_unique<std::stringstream>(content, std::ios::binary);
    } else if constexpr (std::is_same_v<T, fs::path>) {
        auto f = std::make_unique<std::ifstream>(content, std::ios::binary);
        if (!*f) {
            throw std::runtime_error("Failed to open file: " + content.string());
        }
        file = std::move(f); // transfer ownership to polymorphic pointer
    }
    fs::create_directory(gitmini::objectsFolderPath / hash.substr(0, 2));
    std::ofstream outputFile(filePath);

    if (not outputFile.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath.string());
    }
    outputFile << header;
    outputFile << file->rdbuf();
}

//TODO: redo the function in an iterative method to optimize memory.

std::string gitminiHelper::processDirectoryTree(const fs::path &root) {
    std::map<fs::path, std::string> mp;
    std::string treeContent;
    try {
        std::string hash, type;
        for (const auto &entry: fs::directory_iterator(root)) {
            std::string filename = entry.path().filename().string();
            if (!filename.empty() && filename[0] == '.') {
                continue;
            }
            if (entry.is_directory()) {  // Only subdirectories
                hash = gitminiHelper::processDirectoryTree(entry.path());
                type = "tree";
            } else {
                hash = gitminiHelper::hashFile(entry.path(),
                                               ("blob " + std::to_string(fs::file_size(filename)) + '\0'));
                saveObject(hash, entry.path(), ("blob " + std::to_string(fs::file_size(filename)) + '\0'));
                type = "blob";
            }
            treeContent += type + ' ' + filename + ' ' + hash + '\n';

        }
        std::string treeHash = gitminiHelper::hashFile(treeContent,
                                                       "tree " + std::to_string(treeContent.size()) + '\0');
        //TODO: compress the tree object.
        gitminiHelper::saveObject(treeHash, treeContent, "");

        return treeHash;
    } catch (const fs::filesystem_error &e) {
        std::string msg = "process directory error: ";
        msg += e.what();
        throw std::runtime_error(msg);
    }

}

std::string gitminiHelper::structureCommit(const std::map<std::string, std::string> &values) {
    std::string content;
    for (const std::string &element: COMMITELEMENTS) {
        content += element + ' ' + values.at(element) + ' ' + '\n';
    }
    return content;
}

std::string gitminiHelper::objectHeader(const std::string &type, int contentSize) {
    std::string result;
    result = type + ' ' + std::to_string(contentSize) + '\0';
    return result;
}


std::string gitminiHelper::findFileHash(const fs::path &f, const std::string &r) {
    fs::path filePath = f;
    std::string rootHash = r;
    while (true) {

        std::string treeContent = gitminiHelper::readObject(rootHash);

        std::stringstream ss(treeContent);
        std::string type, name, hash;
        bool found = false;
        while (ss >> type) {
            ss >> name;
            ss >> hash;
            if (name == filePath.begin()->string()) {
                found = true;
                break;
            }

        }
        // a new file is created
        if (not found) {
            return "";
        }
        if (type == "blob") {
            return hash;
        }
        std::filesystem::path newPath;

        bool first = true;
        for (const auto &part: filePath) {
            if (first) {
                first = false;
                continue;
            }
            newPath /= part;
        }
        filePath = newPath;
        rootHash = hash;

    }
}

//TODO: test when there is an existing commit tree.
std::string gitminiHelper::readObject(const std::string &hash) {
    fs::path hashPath = gitminiHelper::hashToPath(hash);
    fs::path rootPath = gitmini::objectsFolderPath / hashPath;

    std::ifstream tree(rootPath, std::ios::binary);
    std::vector<char> buffer((std::istreambuf_iterator<char>(tree)),
                             std::istreambuf_iterator<char>());
    std::string content(buffer.data(), buffer.size());
    std::string objectContent;
    if (not content.empty()) {
        objectContent = content.substr(content.find('\0'));
    }
    return objectContent;
}

std::string gitminiHelper::hashToPath(std::string hash) {
    if (hash.size() != (SHA256_DIGEST_LENGTH * 2)) {
        throw std::runtime_error("Error: passed hash is not valid.");
    }
    return hash.substr(0, 2) + '/' + hash.substr(2);
}

// to ignore the SHA256 deprecation

#pragma clang diagnostic pop