#include "file_system.h"

int main(int argc, char* argv[]) {
    FileSystem fileSystem;

    if (argc == 2) {
        json command;
        try {
            command = json::parse(argv[1]);
        } catch (const exception& e) {
            cout << "Error: Invalid JSON input\n";
            return 1;
        }

        if (command.contains("save_state") && command["save_state"] == "true" && command.contains("path")) {
            fileSystem.saveState(command["path"]);
        } else if (command.contains("load_state") && command["load_state"] == "true" && command.contains("path")) {
            fileSystem.loadState(command["path"]);
        } else {
            cout << "Error: Invalid command-line arguments\n";
            return 1;
        }
    } else {
        fileSystem.run();
    }

    return 0;
}
