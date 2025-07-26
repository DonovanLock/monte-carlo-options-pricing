#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include "OptionTypes.h"
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

std::string prepareForOutput(double number) {
    double roundedNumber = std::floor(number * pow(10, NUM_DECIMAL_PLACES_OUTPUT) + 0.5) / pow(10,NUM_DECIMAL_PLACES_OUTPUT);
    std::stringstream stream;
    stream << std::fixed << std::setprecision(NUM_DECIMAL_PLACES_OUTPUT) << roundedNumber;
    return stream.str();
}

void outputRow(std::string key, std::string value, bool rewriteLine) {
    if (rewriteLine) {
        std::cout << "\r";
        std::cout << std::left << std::setw(25) << key << ":" << std::right
        << std::setw(33) << std::fixed << value;
    }
    else {
        std::cout << std::left << std::setw(25) << key << ":" << std::right
        << std::setw(24) << std::fixed << value << std::endl;
    }
}

void outputResults(OptionResult& params) {
    std::cout << std::endl;

    outputRow("Option value", prepareForOutput(params.averagePayoff), false);
    outputRow("Standard error", prepareForOutput(params.standardError), false);

    const std::string roundedLowerBound = prepareForOutput(std::get<0>(params.confidenceInterval));
    const std::string roundedUpperBound = prepareForOutput(std::get<1>(params.confidenceInterval));
    outputRow("95% confidence interval", "[" + roundedLowerBound + ", " + roundedUpperBound + "]", false);

    outputRow("Delta", prepareForOutput(params.greeks.delta), false);
    outputRow("Gamma", prepareForOutput(params.greeks.gamma), false);
    outputRow("Vega", prepareForOutput(params.greeks.vega), false);
    outputRow("Rho", prepareForOutput(params.greeks.rho), false);
    outputRow("Theta", prepareForOutput(params.greeks.theta), false);
}