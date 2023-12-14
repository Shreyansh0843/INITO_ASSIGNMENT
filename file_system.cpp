#include "FileSystem.h"
#include <sstream>
#include <algorithm>
#include <fstream>

using namespace std;

struct FileSystem::Node {
    string name;
    bool isDirectory;
    string content;
    vector<unique_ptr<Node>> children;

    Node(const string& n, bool isDir) : name(n), isDirectory(isDir) {}
};

FileSystem::FileSystem() {
    root = new Node("/", true);
    currentDirectory = root;
}

FileSystem::~FileSystem() {
    deleteFileSystem(root);
}

void FileSystem::mkdir(const string& dirName) {
    auto newDir = make_unique<Node>(dirName, true);
    currentDirectory->children.push_back(move(newDir));
}

void FileSystem::cd(const string& path) {
    Node* targetNode = findNodeByPath(path);
    if (targetNode != nullptr) {
        currentDirectory = targetNode;
    }
}

void FileSystem::ls() const {
    cout << "Contents of " << currentDirectory->name << ":\n";
    if (currentDirectory->children.empty()) {
        cout << "Directory is empty\n";
    } else {
        for (const auto& child : currentDirectory->children) {
            cout << child->name << (child->isDirectory ? "/ " : " ");
        }
        cout << endl;
    }
}

void FileSystem::cat(const string& fileName) const {
    Node* fileNode = findNodeByPath(fileName);
    if (fileNode == nullptr || fileNode->isDirectory) {
        cout << "Error: Invalid file or not a regular file\n";
        return;
    }

    cout << "Contents of " << fileName << ":\n" << fileNode->content << endl;
}

void FileSystem::touch(const string& fileName) {
    auto newFile = make_unique<Node>(fileName, false);
    currentDirectory->children.push_back(move(newFile));
}

void FileSystem::echo(const string& text, const string& fileName) {
    Node* fileNode = findNodeByPath(fileName);
    if (fileNode == nullptr || fileNode->isDirectory) {
        cout << "Error: Invalid file or not a regular file\n";
        return;
    }

    fileNode->content = text;
    cout << "Text written to " << fileName << ": " << text << endl;
}

void FileSystem::mv(const string& source, const string& destination) {
    Node* sourceNode = findNodeByPath(source);
    if (sourceNode == nullptr || sourceNode->isDirectory) {
        cout << "Error: Invalid source or not a regular file\n";
        return;
    }

    Node* destinationNode = findNodeByPath(destination);
    if (destinationNode == nullptr || !destinationNode->isDirectory) {
        cout << "Error: Invalid destination or not a directory\n";
        return;
    }

    auto sourceIt = find_if(destinationNode->children.begin(), destinationNode->children.end(),
                            [sourceNode](const unique_ptr<Node>& node) { return node.get() == sourceNode; });

    if (sourceIt != destinationNode->children.end()) {
        cout << "Error: Destination already contains source\n";
        return;
    }

    if (*sourceIt) {
        auto movedNode = move(*sourceIt);
        destinationNode->children.push_back(move(movedNode));
        currentDirectory->children.erase(
            remove_if(currentDirectory->children.begin(), currentDirectory->children.end(),
                      [sourceNode](const unique_ptr<Node>& node) { return node.get() == sourceNode; }),
            currentDirectory->children.end());
    }
}

void FileSystem::cp(const string& source, const string& destination) {
    Node* sourceNode = findNodeByPath(source);
    if (sourceNode == nullptr || sourceNode->isDirectory) {
        cout << "Error: Invalid source or not a regular file\n";
        return;
    }

    Node* destinationNode = findNodeByPath(destination);
    if (destinationNode == nullptr || !destinationNode->isDirectory) {
        cout << "Error: Invalid destination or not a directory\n";
        return;
    }

    auto copyNode = make_unique<Node>(sourceNode->name, sourceNode->isDirectory);
    copyNode->content = sourceNode->content;
    destinationNode->children.push_back(move(copyNode));
}

void FileSystem::rm(const string& target) {
    Node* targetNode = findNodeByPath(target);
    if (targetNode == nullptr) {
        cout << "Error: Target not found\n";
        return;
    }

    if (targetNode->isDirectory) {
        cout << "Error: Cannot remove a directory, use 'rmdir' instead\n";
        return;
    }

    currentDirectory->children.erase(
        remove_if(currentDirectory->children.begin(), currentDirectory->children.end(),
                  [targetNode](const unique_ptr<Node>& node) { return node.get() == targetNode; }),
        currentDirectory->children.end());
}

void FileSystem::run() {
    cout << "Welcome to the In-Memory File System!\n";
    cout << "Type 'help' for a list of commands.\n";

    string input;
    while (true) {
        cout << currentDirectory->name << "> ";
        getline(cin, input);

        if (input == "exit") {
            break;
        }

        processCommand(input);
    }
}

void FileSystem::saveState(const string& filePath) const {
    json j;
    serializeNode(root, j);
    ofstream file(filePath);
    file << j.dump(4);  // Pretty print with indentation
    file.close();
    cout << "File system state saved to: " << filePath << endl;
}

void FileSystem::loadState(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cout << "Error: Unable to open file: " << filePath << endl;
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const exception& e) {
        cout << "Error: Unable to parse JSON in file: " << filePath << endl;
        return;
    }

    file.close();

    deserializeNode(root, j);
    cout << "File system state loaded from: " << filePath << endl;
}

FileSystem::Node* FileSystem::findNodeByPath(const string& path) {
    if (path.empty() || path[0] != '/') {
        cout << "Error: Invalid path\n";
        return nullptr;
    }

    vector<string> pathComponents;
    stringstream ss(path);
    string component;

    while (getline(ss, component, '/')) {
        if (component == "..") {
            currentDirectory = getParentDirectory(currentDirectory);
        } else {
            auto it = find_if(currentDirectory->children.begin(), currentDirectory->children.end(),
                              [component](const unique_ptr<Node>& node) {
                                  return node->name == component && node->isDirectory;
                              });

            if (it != currentDirectory->children.end()) {
                currentDirectory = it->get();
            } else {
                cout << "Error: Invalid path\n";
                return nullptr;
            }
        }
    }

    return currentDirectory;
}

FileSystem::Node* FileSystem::getParentDirectory(Node* node) const {
    if (node == nullptr || node == root) {
        return nullptr;
    }

    for (const auto& child : root->children) {
        auto it = find_if(child->children.begin(), child->children.end(),
                          [node](const unique_ptr<Node>& n) { return n.get() == node; });
        if (it != child->children.end()) {
            return it->get();
        }
    }

    return nullptr;
}

void FileSystem::processCommand(const string& command) {
    istringstream iss(command);
    string operation;
    iss >> operation;

    if (operation == "mkdir") {
        string dirName;
        iss >> dirName;
        mkdir(dirName);
    } else if (operation == "cd") {
        string path;
        iss >> path;
        cd(path);
    } else if (operation == "ls") {
        ls();
    } else if (operation == "cat") {
        string fileName;
        iss >> fileName;
        cat(fileName);
    } else if (operation == "touch") {
        string fileName;
        iss >> fileName;
        touch(fileName);
    } else if (operation == "echo") {
        string text, fileName;
        iss >> text >> fileName;
        echo(text, fileName);
    } else if (operation == "mv") {
        string source, destination;
        iss >> source >> destination;
        mv(source, destination);
    } else if (operation == "cp") {
        string source, destination;
        iss >> source >> destination;
        cp(source, destination);
    } else if (operation == "rm") {
        string target;
        iss >> target;
        rm(target);
    } else if (operation == "exit") {
        cout << "Exiting the file system. Goodbye!\n";
        exit(0);
    } else {
        cout << "Error: Unknown command\n";
    }
}

void FileSystem::serializeNode(const Node* node, json& j) const {
    j["name"] = node->name;
    j["isDirectory"] = node->isDirectory;
    j["content"] = node->content;

    for (const auto& child : node->children) {
        json childJson;
        serializeNode(child.get(), childJson);
        j["children"].push_back(childJson);
    }
}

void FileSystem::deserializeNode(Node* node, const json& j) {
    node->name = j["name"];
    node->isDirectory = j["isDirectory"];
    node->content = j["content"];

    const auto& childrenJson = j["children"];
    for (const auto& childJson : childrenJson) {
        auto childNode = make_unique<Node>("", false);
        node->children.push_back(move(childNode));
        deserializeNode(node->children.back().get(), childJson);
    }
}

void FileSystem::deleteFileSystem(Node* node) {
    if (node == nullptr) {
        return;
    }

    for (auto& child : node->children) {
        deleteFileSystem(child.get());
    }

    delete node;
}
