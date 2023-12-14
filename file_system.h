#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <memory>

using json = nlohmann::json;

class FileSystem {
private:
    struct Node;

    Node* root;
    Node* currentDirectory;

public:
    FileSystem();
    ~FileSystem();

    void mkdir(const std::string& dirName);
    void cd(const std::string& path);
    void ls() const;
    void cat(const std::string& fileName) const;
    void touch(const std::string& fileName);
    void echo(const std::string& text, const std::string& fileName);
    void mv(const std::string& source, const std::string& destination);
    void cp(const std::string& source, const std::string& destination);
    void rm(const std::string& target);

    void run();
    void saveState(const std::string& filePath) const;
    void loadState(const std::string& filePath);

private:
    Node* findNodeByPath(const std::string& path);
    Node* getParentDirectory(Node* node) const;
    void processCommand(const std::string& command);
    void serializeNode(const Node* node, json& j) const;
    void deserializeNode(Node* node, const json& j);
    void deleteFileSystem(Node* node);
};

#endif // FILESYSTEM_H
