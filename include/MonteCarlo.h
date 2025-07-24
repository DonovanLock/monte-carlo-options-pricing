#pragma once

#include <filesystem>
#include <vector>
#include "OptionTypes.h"

constexpr int NUM_SIMULATIONS = 100000;
constexpr int NUM_GRAPHED_PATHS = 100;
constexpr int NUM_YEARLY_WORKING_DAYS = 252;
constexpr int NUM_YEARLY_DAYS = 365;
constexpr double DT = 1.0 / NUM_YEARLY_WORKING_DAYS;
constexpr double VOLATILITY_JUMP = 0.01;
constexpr double RISK_FREE_RATE_JUMP = 0.001;
constexpr double TIME_TO_MATURITY_JUMP = 1.0 / 365;

std::filesystem::path getWorkingDirectory();

std::vector<std::vector<double>> generateRandomNormals(int numSimulations, int numSteps);

std::tuple<double, double> simulatePath(const OptionParams& params, const std::vector<double>& randomNormals, std::string& graphData, bool graphPath);

double calculatePayoff(const OptionParams& params, std::tuple<double, double> simulatedPrices);

std::vector<double> simulatePayoffs(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, bool graphPaths);

double calculateStandardError(const std::vector<double>& payoffs, double averagePayoff);

std::tuple<double, double> calculateDeltaAndGamma(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice);

double calculateVega(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice);

double calculateRho(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice);

double calculateTheta(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice);

Greeks calculateGreeks(const OptionParams& params, const std::vector<std::vector<double>>& randomNormals, double optionPrice);

OptionResult runMonteCarloSimulation(const OptionParams& params, int numSimulations = NUM_SIMULATIONS);