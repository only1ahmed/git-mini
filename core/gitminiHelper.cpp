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
            (operation == gitminiHelper::stageObject::OPERATION::MODIFY ||
             operation == gitminiHelper::stageObject::OPERATION::CREATE)) {
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
    std::string pathHash;
    std::string result;
    for (const auto &file: files) {
        std::string filePath = file.first.string();
        pathHash = gitminiHelper::hashFile(filePath, "");
        type = file.second.type;
        operation = file.second.operation;
        result += pathHash + ' ' + std::to_string(type) + ' ' + std::to_string(operation) + ' ';
        if (type == gitminiHelper::stageObject::TYPE::FILE &&
            (operation == gitminiHelper::stageObject::OPERATION::MODIFY ||
             operation == gitminiHelper::stageObject::OPERATION::CREATE)) {
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
            file = std::make_unique<std::stringstream>(content);
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

// there is a default value for writingMode which is std::ios::trunc
template<gitminiHelper::StringOrPath T>
void
gitminiHelper::saveObject(const std::string &hash, const T &content, const std::string &header,
                          std::ios_base::openmode writingMode) {

    fs::path hashPath = gitminiHelper::hashToPath(hash);
    fs::path filePath = gitmini::objectsFolderPath / hashPath;

    if (fs::exists(filePath)) return;

    std::unique_ptr<std::istream> file;

    if constexpr (std::is_same_v<T, std::string>) {
        file = std::make_unique<std::stringstream>(content, std::ios::in | std::ios::binary);
    } else if constexpr (std::is_same_v<T, fs::path>) {
        auto f = std::make_unique<std::ifstream>(content, std::ios::binary);
        if (!*f) throw std::runtime_error("Failed to open file: " + content.string());
        file = std::move(f);
    }

    fs::create_directory(gitmini::objectsFolderPath / hash.substr(0, 2));

    std::ofstream outputFile(filePath, std::ios::binary | writingMode);
    if (!outputFile.is_open()) throw std::runtime_error("Cannot open file: " + filePath.string());

    // write header safely
    outputFile.write(header.data(), header.size());

    // write file content safely
//    file->seekg(0, std::ios::beg);
    char buffer[4096];
    while (file->read(buffer, sizeof(buffer)) || file->gcount() > 0) {
        outputFile.write(buffer, file->gcount());
    }

    // optional, but ensures everything is flushed
    outputFile.flush();
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

// TODO: optimize the findFileHash function to start traversing from the given root instead of expecting always the currentCommitRoot
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
        // finally found the file/folder we are searching for...
        if (std::distance(filePath.begin(), filePath.end()) == 1) {
            return hash;
        }
        if (type == std::to_string(gitminiHelper::objectType::BLOB)) {
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
        objectContent = content.substr(content.find('\0') + 1);
    }
    return objectContent;
}

std::string gitminiHelper::hashToPath(std::string hash) {
    if (hash.size() != (SHA256_DIGEST_LENGTH * 2)) {
        throw std::runtime_error("Error: passed hash is not valid.");
    }
    return hash.substr(0, 2) + '/' + hash.substr(2);
}


std::unordered_map<fs::path, gitminiHelper::treeFile> gitminiHelper::readTreeObject(std::string &hash) {
    std::string content = gitminiHelper::readObject(hash);
    std::stringstream ss(content);
    int fileType;
    std::string fileName, fileHash;
    std::unordered_map<fs::path, gitminiHelper::treeFile> result;
    ss.seekg(0);
    while (ss >> fileType) {
        ss >> fileName >> fileHash;
        gitminiHelper::treeFile current;
        current.type = static_cast<gitminiHelper::objectType>(fileType);
        current.name = fileName;
        current.hash = fileHash;
        result[fs::path(fileName)] = current;
    }
    return result;
}

int gitminiHelper::countFilesTreeHash(std::string &hash) {
    std::string content = gitminiHelper::readObject(hash);
    std::stringstream ss(content);
    std::string line;
    int result = 0;
    while (std::getline(ss, line)) {
        result++;
    }
    return result;
}

std::string gitminiHelper::structureTreeObject(std::unordered_map<fs::path, gitminiHelper::treeFile> &treeFiles) {
    std::string result;
    for (auto &file: treeFiles) {
        std::string type = std::to_string(file.second.type);
        std::string name = file.second.name;
        std::string hash = file.second.hash;
        result += type + ' ' + name + ' ' + hash + '\n';

    }
    return result;
}

gitminiHelper::commitObject gitminiHelper::readCommitHash(std::string &hash) {
    std::string content = gitminiHelper::readObject(hash);
    std::stringstream ss(content);
    std::string key, val;
    gitminiHelper::commitObject result;

    std::unordered_map<std::string, std::string> tmpMap;

    while (ss >> key) {
        ss >> val;
        tmpMap[key] = val;
    }
    result.parent = tmpMap["parent"];
    result.root = tmpMap["root"];
    result.message = tmpMap["message"];
    return result;
}

fs::path gitminiHelper::findInParentPath(const fs::path &start, const fs::path &target) {
    fs::path current = start;

    while (true) {
        fs::path candidate = current / target;

        if (fs::exists(candidate))
            return current;

        if (current == current.root_path())
            break;

        current = current.parent_path();
    }

    return {};
}


// Explicit instantiations
template void gitminiHelper::saveObject<std::string>(
        const std::string &hash,
        const std::string &content,
        const std::string &header,
        std::ios_base::openmode

);

template void gitminiHelper::saveObject<std::filesystem::path>(
        const std::string &hash,
        const fs::path &content,
        const std::string &header,
        std::ios_base::openmode
);


template std::string gitminiHelper::hashFile(
        const std::string &content,
        std::string header
);

template std::string gitminiHelper::hashFile(
        const fs::path &content,
        std::string header
);


// to ignore the SHA256 deprecation
#pragma clang diagnostic pop