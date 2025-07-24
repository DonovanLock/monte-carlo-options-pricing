#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <windows.h>
#include "OptionTypes.h"

const int n = 100000; // Number of simulations

OptionResult monteCarloModel(OptionParams params) {

    std::cout << std::endl;

    // Get directory name
    char buffer[_MAX_PATH];
    GetModuleFileNameA(NULL, buffer, _MAX_PATH);
    std::string fullPath(buffer);
    size_t pos = fullPath.find_last_of("\\/");
    std::string workingDirectory = (std::string::npos == pos) ? "" : fullPath.substr(0, pos);

    const int m = 252 * params.timeToMaturity; // Number of time steps
    std::vector<std::vector<double>> randomVariables(n, std::vector<double>(m));
    double payoffSum = 0.0;
    double payoffs[n];
    const int nDigits = (int) log10 ((double) n) + 1;

    // Initialising our random variable generator
    const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(0.0, 1.0);

    // We use the same random variables for our Greeks calculations
    for (int i = 0; i < n; i++) {

        for (int j = 0; j < m; j++) {

            const double Z = distribution(generator); // Z ~ N(0,1)
            randomVariables[i][j] = Z;

        }

    }

    // Creating file to store graph data in
    std::string dataFileName = workingDirectory + "\\" + "graphData.csv";
    std::ofstream GraphData(dataFileName);

    auto approximatePrice = [&payoffs, nDigits, &GraphData](OptionParams params, std::string logText, std::vector<std::vector<double>> randomVariables, double dt, bool originalOption = false) {

        double payoffSum = 0.0;
        const int graphedPaths = 100;

        for (int i = 0; i < randomVariables.size(); i++) {

            std::cout << "\r" + logText + " (" << std::setw (nDigits) << std::to_string(i + 1) + "/" + std::to_string(n) + ")";
            const bool graphPath = i < graphedPaths && originalOption;
            double S = params.spotPrice;
            double antiS = params.spotPrice; // Preparing antithetic path

            for (int j = 0; j < randomVariables.at(0).size(); j++) {

                if (graphPath) {

                    GraphData << std::to_string(S) + ",";

                }

                const double Z = randomVariables[i][j];
                S *= exp((params.riskFreeRate - (pow(params.volatility, 2) / 2.0)) * dt + params.volatility * sqrt(dt) * Z);
                antiS *= exp((params.riskFreeRate - (pow(params.volatility, 2) / 2.0)) * dt + params.volatility * sqrt(dt) * -Z);

            }

            if (graphPath) {

                GraphData << std::to_string(S) + "\n";

            }

            double originalPayoff;
            double antitheticPayoff;

            switch (params.optionType) {

                case OptionType::Call:
                    originalPayoff = std::exp(-params.riskFreeRate * params.timeToMaturity) * (S - params.strikePrice > 0.0 ? S - params.strikePrice : 0.0);
                    antitheticPayoff = std::exp(-params.riskFreeRate * params.timeToMaturity) * (antiS - params.strikePrice > 0.0 ? antiS - params.strikePrice : 0.0);
                    break;

                case OptionType::Put:
                    originalPayoff = std::exp(-params.riskFreeRate * params.timeToMaturity) * (params.strikePrice - S > 0.0 ? params.strikePrice - S : 0.0);
                    antitheticPayoff = std::exp(-params.riskFreeRate * params.timeToMaturity) * (params.strikePrice - antiS > 0.0 ? params.strikePrice - antiS : 0.0);
                    break;

            }

            double payoff = 0.5 * (originalPayoff + antitheticPayoff);

            if (originalOption) {
            
                payoffs[i] = payoff;
            
            }

            payoffSum += payoff;

        }

        const double averagePayoff = payoffSum / n;
        std::cout << std::endl;
        return averagePayoff;

    };

    const double averagePayoff = approximatePrice(params, "Approximating price", randomVariables, params.timeToMaturity / m, true);
    GraphData.close();

    std::string graphFileName = workingDirectory + "\\" + "graphPlotter.py";
    std::string graphCommand = "start /B python3 " + graphFileName;
    system(graphCommand.c_str());
    double squareSum = 0.0;

    for (int i = 0; i < n; i++) {

        squareSum += (payoffs[i] - averagePayoff) * (payoffs[i] - averagePayoff);

    }

    const double sampleVariance = squareSum / (n - 1);
    const double estimatorVariance = sampleVariance / n;
    const double standardError = sqrt(estimatorVariance);

    // Calculating Greeks
    const double deltaS = params.spotPrice * 0.001;
    const double deltaSigma = 0.01;
    const double deltaR = 0.001;
    const double deltaT = 1.0 / 365;

    const OptionParams increasedSpotPriceOption = { params.spotPrice + deltaS, params.strikePrice, params.timeToMaturity, params.riskFreeRate, params.volatility, params.optionType };
    const double increasedSpotPriceOptionValue = approximatePrice(increasedSpotPriceOption, "Calculating delta", randomVariables, params.timeToMaturity / m);
    const double delta = (increasedSpotPriceOptionValue - averagePayoff) / deltaS;

    const OptionParams decreasedSpotPriceOption = { params.spotPrice - deltaS, params.strikePrice, params.timeToMaturity, params.riskFreeRate, params.volatility, params.optionType };
    const double decreasedSpotPriceOptionValue = approximatePrice(decreasedSpotPriceOption, "Calculating gamma", randomVariables, params.timeToMaturity / m);
    const double gamma = (increasedSpotPriceOptionValue - (2 * averagePayoff) + decreasedSpotPriceOptionValue) / pow(deltaS, 2);

    const OptionParams increasedVolatilityOption = { params.spotPrice, params.strikePrice, params.timeToMaturity, params.riskFreeRate, params.volatility + deltaSigma, params.optionType };
    const double increasedVolatilityOptionValue = approximatePrice(increasedVolatilityOption, "Calculating vega", randomVariables, params.timeToMaturity / m);
    const double vega = (increasedVolatilityOptionValue - averagePayoff) / deltaSigma;

    const OptionParams increasedInterestRateOption = { params.spotPrice, params.strikePrice, params.timeToMaturity, params.riskFreeRate + deltaR, params.volatility, params.optionType };
    const double increasedInterestRateOptionValue = approximatePrice(increasedInterestRateOption, "Calculating rho", randomVariables, params.timeToMaturity / m);
    const double rho = (increasedInterestRateOptionValue - averagePayoff) / deltaR;

    const OptionParams increasedExpirationTimeOption = { params.spotPrice, params.strikePrice, params.timeToMaturity + deltaT, params.riskFreeRate, params.volatility, params.optionType };
    const int increasedNumberOfSteps = 252 * increasedExpirationTimeOption.timeToMaturity;
    std::vector<std::vector<double>> randomVariablesExtended(n, std::vector<double>(increasedNumberOfSteps));

    for (int i = 0; i < n; i++) {

        for (int j = 0; j < increasedNumberOfSteps; j++) {

            if (j < m) {

                randomVariablesExtended[i][j] = randomVariables[i][j];

            }

            else {

                const double Z = distribution(generator); // Z ~ N(0,1)
                randomVariablesExtended[i][j] = Z;

            }

        }

    }

    const double increasedExpirationTimeOptionValue = approximatePrice(increasedExpirationTimeOption, "Calculating theta", randomVariablesExtended, (params.timeToMaturity + deltaT) / m);
    const double theta = -((increasedExpirationTimeOptionValue - averagePayoff) / deltaT);

    OptionResult model;
    model.averagePayoff = averagePayoff;
    model.standardError = standardError;
    model.greeks = { delta, gamma, vega, rho, theta };

    return model;

}

std::string modelToString(std::string name, OptionResult model) {

    std::string output = name + ":\n\n";
    output += "  Estimated Option Price | " + std::to_string(model.averagePayoff) + "\n";
    output += "          Standard Error | " + std::to_string(model.standardError) + "\n";

    const double confidenceMargin = 1.96 * model.standardError;
    const double lowerBound = model.averagePayoff - confidenceMargin;
    const double upperBound = model.averagePayoff + confidenceMargin;

    output += "95% probability interval | (" + std::to_string(lowerBound) + ", " + std::to_string(upperBound) + ")\n\n";
    output += "                   Delta | " + std::to_string(model.greeks.delta) + "\n";
    output += "                   Gamma | " + std::to_string(model.greeks.gamma) + "\n";
    output += "                    Vega | " + std::to_string(model.greeks.vega) + "\n";
    output += "                     Rho | " + std::to_string(model.greeks.rho) + "\n";
    output += "                   Theta | " + std::to_string(model.greeks.theta) + "\n";

    return output;

}

int main() {

    OptionParams amazonOption { 226.13, 235, 1.164, 0.044, 0.2866, OptionType::Call };
    OptionResult amazonModel = monteCarloModel(amazonOption); // Expecting ~38
    std::cout << std::endl << modelToString("Call option", amazonModel);
    return 0;

}
