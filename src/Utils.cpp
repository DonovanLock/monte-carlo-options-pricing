#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include "OptionTypes.h"
#include "Utils.h"

void outputHelp() {
    std::cout << "=================================\n"
              << " Monte Carlo Option Pricing Tool \n"
              << "=================================\n"
              << "Usage:\n"
              << "  ./MonteCarlo.exe [options]\n\n"
              << "Options:\n"
              << "  -h     Show this help message and exit\n"
              << "  -d     Run demo simulation with example Amazon option parameters\n"
              << "  [spotPrice] [strikePrice] [timeToMaturity] [riskFreeRate] [volatility] [optionType]\n"
              << "         Run simulation with user-specified parameters:\n"
              << "           spotPrice        Spot price (positive double)\n"
              << "           strikePrice      Strike price (positive double)\n"
              << "           timeToMaturity   Time to maturity in years (positive double)\n"
              << "           riskFreeRate     Risk-free interest rate as percentage (non-negative double)\n"
              << "           volatility       Volatility as percentage (non-negative double)\n"
              << "           optionType       Option type: Call or Put (case-insensitive)\n\n"
              << "If no arguments are provided, the program will prompt interactively for these inputs.\n\n"
              << std::endl;
}

bool isPositiveDouble(const char* price) {
    if (price == nullptr) {
        return false;
    }
    std::string priceString(price);
    if (priceString.empty()) {
        return false;
    }

    try {
        size_t position;
        double priceValue = std::stod(priceString, &position);
        if (position != priceString.length() || priceValue <= 0) {
            return false;
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

bool isNonNegativeDouble(const char* price) {
    if (price == nullptr) {
        return false;
    }
    std::string priceString(price);
    if (priceString.empty()) {
        return false;
    }

    try {
        size_t position;
        double priceValue = std::stod(priceString, &position);
        if (position != priceString.length() || priceValue < 0) {
            return false;
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

bool insensitiveEquals(std::string string1, std::string string2) {
    if (string1.length() != string2.length()) {
        return false;
    }
    for (int i = 0; i < string1.length(); i++) {
        if (tolower(string1[i]) != tolower(string2[i])) {
            return false;
        }
    }
    return true;
}

bool isValidOptionType(const char* optionType) {
    std::string optionString(optionType);
    return insensitiveEquals(optionString, "Call") || insensitiveEquals(optionString, "Put");
}

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

void outputRow(std::string key, std::string value) {
    std::cout << std::right << std::setw(25) << key << " : " << std::left
    << std::setw(24) << std::fixed << value << std::endl;
}

void outputResults(OptionResult& params) {
    std::cout << std::endl;

    outputRow("Option value", prepareForOutput(params.averagePayoff));
    outputRow("Standard error", prepareForOutput(params.standardError));

    const std::string roundedLowerBound = prepareForOutput(std::get<0>(params.confidenceInterval));
    const std::string roundedUpperBound = prepareForOutput(std::get<1>(params.confidenceInterval));
    outputRow("95% confidence interval", "(" + roundedLowerBound + ", " + roundedUpperBound + ")");

    outputRow("Delta", prepareForOutput(params.greeks.delta));
    outputRow("Gamma", prepareForOutput(params.greeks.gamma));
    outputRow("Vega", prepareForOutput(params.greeks.vega));
    outputRow("Rho", prepareForOutput(params.greeks.rho));
    outputRow("Theta", prepareForOutput(params.greeks.theta));
}