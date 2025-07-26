#include <iostream>
#include <string>
#include "MonteCarlo.h"
#include "Utils.h"

int main(int argc, char* argv[]) {
    if (argc == 2 && strcmp("-h", argv[1]) == 0) {
        outputHelp();
    }
    else if (argc == 2 && strcmp("-d", argv[1]) == 0) {
        OptionParams amazonOption { 226.13, 235, 1.164, 0.044, 0.2866, OptionType::Call };
        OptionResult amazonModel = runMonteCarloSimulation(amazonOption);
        outputResults(amazonModel);
    }
    else if (argc == 7) {
        if (!isPositiveDouble(argv[1])) {
            std::cerr << "\033[31mERROR: The spot price must be a positive double.\033[0m";
            return EXIT_FAILURE;
        }
        else if (!isPositiveDouble(argv[2])) {
            std::cerr << "\033[31mERROR: The strike price must be a positive double.\033[0m";
            return EXIT_FAILURE;
        }
        else if (!isPositiveDouble(argv[3])) {
            std::cerr << "\033[31mERROR: The time to maturity must be a positive double.\033[0m";
            return EXIT_FAILURE;
        }
        else if (!isNonNegativeDouble(argv[4])) {
            std::cerr << "\033[31mERROR: The risk-free rate must be a non-negative double.\033[0m";
            return EXIT_FAILURE;
        }
        else if (!isNonNegativeDouble(argv[5])) {
            std::cerr << "\033[31mERROR: The volatility must be a non-negative double.\033[0m";
            return EXIT_FAILURE;
        }
        else if (!isValidOptionType(argv[6])) {
            std::cerr << "\033[31mERROR: The option type must be either \"Call\" or \"Put\".\033[0m";
            return EXIT_FAILURE;
        }
        else {
            OptionType optionType;
            if (insensitiveEquals(argv[6], "Call")) {
                optionType = OptionType::Call;
            }
            else {
                optionType = OptionType::Put;
            }
            OptionParams option = { std::stod(argv[1]), std::stod(argv[2]), std::stod(argv[3]), std::stod(argv[4]) / 100.0, std::stod(argv[5]) / 100.0, optionType };
            OptionResult model = runMonteCarloSimulation(option);
            outputResults(model);
        }
    }
    else if (argc == 1) {
        std::string spotPriceString, strikePriceString, timeToMaturityString, riskFreeRateString, volatilityString, optionTypeString;
        std::cout << "Enter the spot price: ";
        std::cin >> spotPriceString;
        while (!isPositiveDouble(spotPriceString.c_str())) {
            std::cout << "The spot price must be a positive double." << std::endl;
            std::cout << "Enter the spot price: ";
            std::cin >> spotPriceString;
        }

        std::cout << "Enter the strike price: ";
        std::cin >> strikePriceString;
        while (!isPositiveDouble(strikePriceString.c_str())) {
            std::cout << "The strike price must be a positive double." << std::endl;
            std::cout << "Enter the strike price: ";
            std::cin >> strikePriceString;
        }

        std::cout << "Enter the time to maturity in years: ";
        std::cin >> timeToMaturityString;
        while (!isPositiveDouble(timeToMaturityString.c_str())) {
            std::cout << "The time to maturity must be a positive double." << std::endl;
            std::cout << "Enter the time to maturity in years: ";
            std::cin >> timeToMaturityString;
        }

        std::cout << "Enter the risk-free rate as a percentage: ";
        std::cin >> riskFreeRateString;
        while (!isNonNegativeDouble(riskFreeRateString.c_str())) {
            std::cout << "The risk-free rate must be a non-negative double." << std::endl;
            std::cout << "Enter the risk-free rate as a percentage: ";
            std::cin >> riskFreeRateString;
        }

        std::cout << "Enter the volatility as a percentage: ";
        std::cin >> volatilityString;
        while (!isNonNegativeDouble(volatilityString.c_str())) {
            std::cout << "The volatility must be a non-negative double." << std::endl;
            std::cout << "Enter the volatility as a percentage: ";
            std::cin >> volatilityString;
        }

        std::cout << "Enter the option type (Call/Put): ";
        std::cin >> optionTypeString;
        while (!isValidOptionType(optionTypeString.c_str())) {
            std::cout << "The option type must be either \"Call\" or \"Put\"." << std::endl;
            std::cout << "Enter the option type (Call/Put): ";
            std::cin >> optionTypeString;
        }

        OptionType optionType;
        if (insensitiveEquals(optionTypeString, "Call")) {
            optionType = OptionType::Call;
        }
        else {
            optionType = OptionType::Put;
        }

        OptionParams option = { std::stod(spotPriceString), std::stod(strikePriceString), std::stod(timeToMaturityString), std::stod(riskFreeRateString) / 100.0, std::stod(volatilityString) / 100.0, optionType };
        OptionResult model = runMonteCarloSimulation(option);
        outputResults(model);
    }
    else {
        std::cerr << "Unrecognised argument format provided. Use ./MonteCarlo.exe -h for instructions.";
    }
    return 0;
}