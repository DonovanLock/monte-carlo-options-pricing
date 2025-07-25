#include <filesystem>
#include <iostream>
#include <thread>
#include "Utils.h"

std::filesystem::path getRootDirectory() {
    const std::filesystem::path filePath = __FILE__;
    return filePath.parent_path().parent_path();
}

std::string buildPythonCommand(const std::filesystem::path& scriptPath) {
    #ifdef _WIN32
        return "python " + scriptPath.string() + " > nul 2>&1";
    #else
        return "python3 " + scriptPath.string() + " > /dev/null 2>&1";
    #endif
}

void runGraphPlotter() {
    try {
        std::filesystem::path scriptPath = getRootDirectory() / "scripts" / "graphPlotter.py";
        if (!std::filesystem::exists(scriptPath)) {
            std::cerr << "Error: graphPlotter.py script not found at " << scriptPath << "." << std::endl;
            return;
        }
        std::string command = buildPythonCommand(scriptPath);
        std::thread asyncThread([command]() {
            int result = std::system(command.c_str());
            if (result != 0) {
                std::cerr << "Error: Python script exited with code " << result << "." << std::endl;
            }
        });

        asyncThread.detach();
    }
    catch (const std::exception& error) {
        std::cerr << "Exception while launching graphPlotter.py: " << error.what() << "." << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error while launching graphPlotter.py." << std::endl;
    }
}