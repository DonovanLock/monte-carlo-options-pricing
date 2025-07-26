#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include "MonteCarlo.h"
#include "Utils.h"

std::vector<std::vector<double>> generateRandomNormals(int numSimulations, int numSteps) {
    std::vector<std::vector<double>> randomNormals(numSimulations, std::vector<double>(numSteps));
    
    // Initialising our random variable generator
    const unsigned seed = unsigned int (std::chrono::system_clock::now().time_since_epoch().count());
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(0.0, 1.0);
    
    // We use the same random variables for our Greeks calculations
    for (int i = 0; i < numSimulations; i++) {
        for (int j = 0; j < numSteps; j++) {
            const double Z = distribution(generator); // Z ~ N(0,1)
            randomNormals[i][j] = Z;
        }
    }

    return randomNormals;
}

std::tuple<double, double> simulatePath(const OptionParams& params, const std::vector<double>& randomNormals, std::string& graphData, bool graphPath) {
    double price, antiPrice;
    price = antiPrice = params.spotPrice;

    for (double Z : randomNormals) {
        if (graphPath) {
            graphData += std::to_string(price) + ",";
        }
        price *= exp((params.riskFreeRate - (pow(params.volatility, 2) / 2.0))
         * DT + params.volatility * sqrt(DT) * Z);
        antiPrice *= exp((params.riskFreeRate - (pow(params.volatility, 2) / 2.0))
         * DT + params.volatility * sqrt(DT) * -Z);
    }

    if (graphPath) {
        graphData += std::to_string(price) + "\n";
    }

    return std::make_tuple(price, antiPrice);
}

double calculatePayoff(const OptionParams& params, std::tuple<double, double> simulatedPrices) {
    double finalPrice = std::get<0>(simulatedPrices);
    double finalAntitheticPrice = std::get<1>(simulatedPrices);
    double payoff, antitheticPayoff;
    payoff = antitheticPayoff = 0.0;

    switch (params.optionType) {
        case OptionType::Call:
            payoff = std::exp(-params.riskFreeRate * params.timeToMaturity)
             * (finalPrice - params.strikePrice > 0.0 ? finalPrice - params.strikePrice : 0.0);
            antitheticPayoff = std::exp(-params.riskFreeRate * params.timeToMaturity)
             * (finalAntitheticPrice - params.strikePrice > 0.0 ? finalAntitheticPrice - params.strikePrice : 0.0);
            break;
        case OptionType::Put:
            payoff = std::exp(-params.riskFreeRate * params.timeToMaturity)
             * (params.strikePrice - finalPrice > 0.0 ? params.strikePrice - finalPrice : 0.0);
            antitheticPayoff = std::exp(-params.riskFreeRate * params.timeToMaturity)
             * (params.strikePrice - finalAntitheticPrice > 0.0 ? params.strikePrice - finalAntitheticPrice : 0.0);
            break;
    }

    return 0.5 * (payoff + antitheticPayoff);
}

std::vector<double> simulatePayoffs(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, bool graphPaths, std::string logText) {
    int numSimulations = int (randomNormals.size());
    std::vector<double> payoffSamples(numSimulations);
    std::string graphData = "";
    int percentageComplete = 0;

    for (int i = 0; i < numSimulations; i++) {
        if (percentageComplete < (int) (i) * 100 / numSimulations) {
            percentageComplete = (i) * 100 / numSimulations;
            outputRow(logText, "\033[33m" + std::to_string(percentageComplete) + "%\033[0m", true);
        }
        std::tuple<double, double> finalPrices = simulatePath(params, randomNormals[i], graphData, i < NUM_GRAPHED_PATHS && graphPaths);
        payoffSamples[i] = calculatePayoff(params, finalPrices);
    }
    outputRow(logText, "\033[32m100%\033[0m", true);
    std::cout << std::endl;

    if (graphPaths) {
        std::filesystem::path outputDirectory = getRootDirectory() / "output";
        std::filesystem::create_directories(outputDirectory);
        std::filesystem::path graphDataFileName = outputDirectory / "graphData.csv";
        std::ofstream graphDataFile(graphDataFileName);
        graphDataFile << graphData;
        graphDataFile.close();
        runGraphPlotter();
    }

    return payoffSamples;
}

double calculateStandardError(const std::vector<double>& payoffs, double averagePayoff) {
    double squareSum = 0.0;
    for (double payoff : payoffs) {
        squareSum += (payoff - averagePayoff) * (payoff - averagePayoff);
    }
    const double sampleVariance = squareSum / (NUM_SIMULATIONS - 1);
    const double estimatorVariance = sampleVariance / NUM_SIMULATIONS;
    return sqrt(estimatorVariance);
}

std::tuple<double,double> calculateConfidenceInterval(double averagePayoff, double standardError) {
    const double lowerBound = averagePayoff - (standardError * CONFIDENCE_BOUND_FACTOR);
    const double upperBound = averagePayoff + (standardError * CONFIDENCE_BOUND_FACTOR);
    return std::make_tuple(lowerBound, upperBound);
}

std::tuple<double, double> calculateDeltaAndGamma(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice) {
    const double spotPriceJump = params.spotPrice * 0.001;
    OptionParams spotPriceUpOption = params;
    spotPriceUpOption.spotPrice = params.spotPrice + spotPriceJump;
    std::vector<double> spotPriceUpPayoffSamples = simulatePayoffs(spotPriceUpOption, randomNormals, false, "Calculating delta");
    double averageSpotPriceUpPayoff = std::accumulate(spotPriceUpPayoffSamples.begin(), spotPriceUpPayoffSamples.end(), 0.0) / spotPriceUpPayoffSamples.size();
    double delta = (averageSpotPriceUpPayoff - optionPrice) / spotPriceJump;

    OptionParams spotPriceDownOption = params;
    spotPriceDownOption.spotPrice = params.spotPrice - spotPriceJump;
    std::vector<double> spotPriceDownPayoffSamples = simulatePayoffs(spotPriceDownOption, randomNormals, false, "Calculating gamma");
    double averageSpotPriceDownPayoff = std::accumulate(spotPriceDownPayoffSamples.begin(), spotPriceDownPayoffSamples.end(), 0.0) / spotPriceDownPayoffSamples.size();
    double gamma = (averageSpotPriceUpPayoff - (2 * optionPrice) + averageSpotPriceDownPayoff) / pow(spotPriceJump, 2);

    return std::make_tuple(delta, gamma);
}

double calculateVega(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice) {
    OptionParams volatilityUpOption = params;
    volatilityUpOption.volatility = params.volatility + VOLATILITY_JUMP;
    std::vector<double> volatilityUpPayoffSamples = simulatePayoffs(volatilityUpOption, randomNormals, false, "Calculating vega");
    double averageVolatilityUpPayoff = std::accumulate(volatilityUpPayoffSamples.begin(), volatilityUpPayoffSamples.end(), 0.0) / volatilityUpPayoffSamples.size();
    double vega = (averageVolatilityUpPayoff - optionPrice) / VOLATILITY_JUMP;

    return vega;
}

double calculateRho(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice) {
    OptionParams riskFreeRateUpOption = params;
    riskFreeRateUpOption.riskFreeRate = params.riskFreeRate + RISK_FREE_RATE_JUMP;
    std::vector<double> riskFreeRateUpPayoffSamples = simulatePayoffs(riskFreeRateUpOption, randomNormals, false, "Calculating rho");
    double averageRiskFreeRateUpPayoff = std::accumulate(riskFreeRateUpPayoffSamples.begin(), riskFreeRateUpPayoffSamples.end(), 0.0) / riskFreeRateUpPayoffSamples.size();
    double rho = (averageRiskFreeRateUpPayoff - optionPrice) / RISK_FREE_RATE_JUMP;

    return rho;
}

double calculateTheta(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice) {
    OptionParams timeToMaturityUpOption = params;
    timeToMaturityUpOption.timeToMaturity = params.timeToMaturity + TIME_TO_MATURITY_JUMP;

    const int numSteps = int (randomNormals[0].size());
    const int increasedNumSteps = int (NUM_YEARLY_WORKING_DAYS * timeToMaturityUpOption.timeToMaturity);

    std::vector<std::vector<double>> extraRandomNormals = generateRandomNormals(NUM_SIMULATIONS, increasedNumSteps - numSteps);
    std::vector<std::vector<double>> randomNormalsExtended(NUM_SIMULATIONS, std::vector<double>(increasedNumSteps));
    for (int i = 0; i < NUM_SIMULATIONS; i++) {
        for (int j = 0; j < increasedNumSteps; j++) {
            if (j < numSteps) {
                randomNormalsExtended[i][j] = randomNormals[i][j];
            }
            else {
                randomNormalsExtended[i][j] = extraRandomNormals[i][j - numSteps];
            }
        }
    }

    std::vector<double> timeToMaturityUpPayoffSamples = simulatePayoffs(timeToMaturityUpOption, randomNormalsExtended, false, "Calculating theta");
    double averageTimeToMaturityUpPayoff = std::accumulate(timeToMaturityUpPayoffSamples.begin(), timeToMaturityUpPayoffSamples.end(), 0.0) / timeToMaturityUpPayoffSamples.size();
    double theta = -(averageTimeToMaturityUpPayoff - optionPrice) / TIME_TO_MATURITY_JUMP;

    return theta;
}

Greeks calculateGreeks(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice) {
    double delta;
    double gamma;
    std::tie(delta, gamma) = calculateDeltaAndGamma(params, randomNormals, optionPrice);
    const double vega = calculateVega(params, randomNormals, optionPrice);
    const double rho = calculateRho(params, randomNormals, optionPrice);
    const double theta = calculateTheta(params, randomNormals, optionPrice);
    return { delta, gamma, vega, rho, theta };
}

OptionResult runMonteCarloSimulation(const OptionParams& params) {
    const int numSteps = int (params.timeToMaturity * NUM_YEARLY_WORKING_DAYS);
    std::vector<std::vector<double>> randomNormals = generateRandomNormals(NUM_SIMULATIONS, numSteps);
    std::vector<double> payoffSamples = simulatePayoffs(params, randomNormals, true, "Simulating paths");

    const double averagePayoff = std::accumulate(payoffSamples.begin(), payoffSamples.end(), 0.0) / payoffSamples.size();
    const double standardError = calculateStandardError(payoffSamples, averagePayoff);
    const std::tuple<double, double> confidenceInterval = calculateConfidenceInterval(averagePayoff, standardError);
    const Greeks greeks = calculateGreeks(params, randomNormals, averagePayoff);

    return { averagePayoff, standardError, confidenceInterval, greeks };
}
