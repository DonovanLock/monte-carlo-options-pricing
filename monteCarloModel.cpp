#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <windows.h>

const enum class OptionType { Call, Put };

const struct OptionParams {

    double S0; // Spot price
    double K; // Strike/Exercise price
    double T; // Time to expiry
    double r; // Risk-free interest rate
    double sigma; // Volatility
    OptionType optiontype; // Call or Sell

};

const struct Greeks {

    double delta;
    double gamma;
    double vega;
    double rho;
    double theta;

};

const struct OptionModel {

    double averagePayoff;
    double standardError;
    Greeks greeks;

};

const int n = 100000; // Number of simulations

OptionModel monteCarloModel(OptionParams params) {

    std::cout << std::endl;

    // Get directory name
    char buffer[_MAX_PATH];
    GetModuleFileNameA(NULL, buffer, _MAX_PATH);
    std::string fullPath(buffer);
    size_t pos = fullPath.find_last_of("\\/");
    std::string workingDirectory = (std::string::npos == pos) ? "" : fullPath.substr(0, pos);

    const int m = 252 * params.T; // Number of time steps
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
            double S = params.S0;
            double antiS = params.S0; // Preparing antithetic path

            for (int j = 0; j < randomVariables.at(0).size(); j++) {

                if (graphPath) {

                    GraphData << std::to_string(S) + ",";

                }

                const double Z = randomVariables[i][j];
                S *= exp((params.r - (pow(params.sigma, 2) / 2.0)) * dt + params.sigma * sqrt(dt) * Z);
                antiS *= exp((params.r - (pow(params.sigma, 2) / 2.0)) * dt + params.sigma * sqrt(dt) * -Z);

            }

            if (graphPath) {

                    GraphData << std::to_string(S) + "\n";

            }

            double originalPayoff;
            double antitheticPayoff;

            switch (params.optiontype) {

                case OptionType::Call:
                    originalPayoff = std::exp(-params.r * params.T) * (S - params.K > 0.0 ? S - params.K : 0.0);
                    antitheticPayoff = std::exp(-params.r * params.T) * (antiS - params.K > 0.0 ? antiS - params.K : 0.0);
                    break;

                case OptionType::Put:
                    originalPayoff = std::exp(-params.r * params.T) * (params.K - S > 0.0 ? params.K - S : 0.0);
                    antitheticPayoff = std::exp(-params.r * params.T) * (params.K - antiS > 0.0 ? params.K - antiS : 0.0);
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

    const double averagePayoff = approximatePrice(params, "Approximating price", randomVariables, params.T / m, true);
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
    const double deltaS = params.S0 * 0.001;
    const double deltaSigma = 0.01;
    const double deltaR = 0.001;
    const double deltaT = 1.0 / 365;

    const OptionParams increasedSpotPriceOption = { params.S0 + deltaS, params.K, params.T, params.r, params.sigma, params.optiontype };
    const double increasedSpotPriceOptionValue = approximatePrice(increasedSpotPriceOption, "Calculating delta", randomVariables, params.T / m);
    const double delta = (increasedSpotPriceOptionValue - averagePayoff) / deltaS;

    const OptionParams decreasedSpotPriceOption = { params.S0 - deltaS, params.K, params.T, params.r, params.sigma, params.optiontype };
    const double decreasedSpotPriceOptionValue = approximatePrice(decreasedSpotPriceOption, "Calculating gamma", randomVariables, params.T / m);
    const double gamma = (increasedSpotPriceOptionValue - (2 * averagePayoff) + decreasedSpotPriceOptionValue) / pow(deltaS, 2);

    const OptionParams increasedVolatilityOption = { params.S0, params.K, params.T, params.r, params.sigma + deltaSigma, params.optiontype };
    const double increasedVolatilityOptionValue = approximatePrice(increasedVolatilityOption, "Calculating vega", randomVariables, params.T / m);
    const double vega = (increasedVolatilityOptionValue - averagePayoff) / deltaSigma;

    const OptionParams increasedInterestRateOption = { params.S0, params.K, params.T, params.r + deltaR, params.sigma, params.optiontype };
    const double increasedInterestRateOptionValue = approximatePrice(increasedInterestRateOption, "Calculating rho", randomVariables, params.T / m);
    const double rho = (increasedInterestRateOptionValue - averagePayoff) / deltaR;

    const OptionParams increasedExpirationTimeOption = { params.S0, params.K, params.T + deltaT, params.r, params.sigma, params.optiontype };
    const int increasedNumberOfSteps = 252 * increasedExpirationTimeOption.T;
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

    const double increasedExpirationTimeOptionValue = approximatePrice(increasedExpirationTimeOption, "Calculating theta", randomVariablesExtended, (params.T + deltaT) / m);
    const double theta = -((increasedExpirationTimeOptionValue - averagePayoff) / deltaT);

    OptionModel model;
    model.averagePayoff = averagePayoff;
    model.standardError = standardError;
    model.greeks = { delta, gamma, vega, rho, theta };

    return model;

}

std::string modelToString(std::string name, OptionModel model) {

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
    OptionModel amazonModel = monteCarloModel(amazonOption); // Expecting ~38
    std::cout << std::endl << modelToString("Call option", amazonModel);
    return 0;

}
