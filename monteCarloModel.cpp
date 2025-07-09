#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

enum class OptionType { Call, Put };

struct OptionParams {

    double S0; // Spot price
    double K; // Strike/Exercise price
    double T; // Time to expiry
    double r; // Risk-free interest rate
    double sigma; // Volatility
    OptionType optiontype; // Call or Sell

};

struct Greeks {

    double delta;
    double gamma;
    double vega;
    double rho;
    double theta;

};

struct OptionModel {

    double averagePayoff;
    double standardError;
    Greeks greeks;

};

const int n = 100000; // Number of simulations

/*OptionModel monteCarloModel(OptionParams params) {

    const int m = 252 * params.T; // Number of time steps
    
    double payoffSum = 0.0;
    double payoffs[n];
    double dt = params.T / m;

    // Initialising our random variable generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(0.0, 1.0);

    for (int i = 0; i < n; i++) {

        double S = params.S0;

        for (int j = 0; j < m; j++) {

            double Z = distribution(generator); // Z ~ N(0,1)
            S *= exp((params.r - (pow(params.sigma, 2) / 2.0)) * dt + params.sigma * sqrt(dt) * Z);

        }

        double payoff;

        switch (params.optiontype) {

            case OptionType::Call:
                payoff = exp(-params.r * params.T) * std::max(S - params.K, 0.0);
                break;
            
            case OptionType::Put:
                payoff = exp(-params.r * params.T) * std::max(params.K - S, 0.0);
                break;

        }

        payoffs[i] = payoff;
        payoffSum += payoff;

    }

    double averagePayoff = payoffSum / n;
    double squareSum = 0.0;

    for (int i = 0; i < n; i++) {
        squareSum += (payoffs[i] - averagePayoff) * (payoffs[i] - averagePayoff);
    }

    double sampleVariance = squareSum / (n - 1);
    double estimatorVariance = sampleVariance / n;
    double standardError = sqrt(estimatorVariance);

    return { averagePayoff, standardError };

}*/

OptionModel monteCarloModel(OptionParams params) {

    const int m = 252 * params.T; // Number of time steps
    std::vector<std::vector<double>> randomVariables(n, std::vector<double>(m));
    double payoffSum = 0.0;
    double payoffs[n];

    // Initialising our random variable generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(0.0, 1.0);

    // We use the same random variables for our Greeks calculations
    for (int i = 0; i < n; i++) {

        for (int j = 0; j < m; j++) {

            double Z = distribution(generator); // Z ~ N(0,1)
            randomVariables[i][j] = Z;

        }

    }

    auto approximatePrice = [&payoffs](OptionParams params, std::vector<std::vector<double>> randomVariables, double dt, bool overwritePayoffs = false) {

        double payoffSum = 0.0;

        for (int i = 0; i < randomVariables.size(); i++) {

            double S = params.S0;

            for (int j = 0; j < randomVariables.at(0).size(); j++) {

                S *= exp((params.r - (pow(params.sigma, 2) / 2.0)) * dt + params.sigma * sqrt(dt) * randomVariables[i][j]);

            }

            double payoff;

            switch (params.optiontype) {

                case OptionType::Call:
                    payoff = exp(-params.r * params.T) * std::max(S - params.K, 0.0);
                    break;
                
                case OptionType::Put:
                    payoff = exp(-params.r * params.T) * std::max(params.K - S, 0.0);
                    break;

            }

            payoffs[i] = payoff;
            payoffSum += payoff;

        }

        double averagePayoff = payoffSum / n;
        return averagePayoff;
    
    };

    double averagePayoff = approximatePrice(params, randomVariables, params.T / m, true);
    double squareSum = 0.0;

    for (int i = 0; i < n; i++) {

        squareSum += (payoffs[i] - averagePayoff) * (payoffs[i] - averagePayoff);

    }

    double sampleVariance = squareSum / (n - 1);
    double estimatorVariance = sampleVariance / n;
    double standardError = sqrt(estimatorVariance);

    // Calculating Greeks
    const double deltaS = params.S0 * 0.001;
    const double deltaSigma = 0.01;
    const double deltaR = 0.001;
    const double deltaT = 1.0 / 365;

    const OptionParams increasedSpotPriceOption = { params.S0 + deltaS, params.K, params.T, params.r, params.sigma, params.optiontype };
    const double increasedSpotPriceOptionValue = approximatePrice(increasedSpotPriceOption, randomVariables, params.T / m);
    const double delta = (increasedSpotPriceOptionValue - averagePayoff) / deltaS;

    const OptionParams decreasedSpotPriceOption = { params.S0 - deltaS, params.K, params.T, params.r, params.sigma, params.optiontype };
    const double decreasedSpotPriceOptionValue = approximatePrice(decreasedSpotPriceOption, randomVariables, params.T / m);
    const double gamma = (increasedSpotPriceOptionValue - (2 * averagePayoff) + decreasedSpotPriceOptionValue) / pow(deltaS, 2);

    const OptionParams increasedVolatilityOption = { params.S0, params.K, params.T, params.r, params.sigma + deltaSigma, params.optiontype };
    const double increasedVolatilityOptionValue = approximatePrice(increasedVolatilityOption, randomVariables, params.T / m);
    const double vega = (increasedVolatilityOptionValue - averagePayoff) / deltaSigma;

    const OptionParams increasedInterestRateOption = { params.S0, params.K, params.T, params.r + deltaR, params.sigma, params.optiontype };
    const double increasedInterestRateOptionValue = approximatePrice(increasedInterestRateOption, randomVariables, params.T / m);
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

                double Z = distribution(generator); // Z ~ N(0,1)
                randomVariablesExtended[i][j] = Z;

            }

        }

    }

    const double increasedExpirationTimeOptionValue = approximatePrice(increasedExpirationTimeOption, randomVariablesExtended, (params.T + deltaT) / m);
    const double theta = -((increasedExpirationTimeOptionValue - averagePayoff) / deltaT);

    OptionModel model;
    model.averagePayoff = averagePayoff;
    model.standardError = standardError;
    model.greeks = { delta, gamma, vega, rho, theta };

    return model;

}

std::string modelToString(std::string name, OptionModel model) {

    std::string output = "\n" + name + ":\n";
    output += "  Estimated Option Price: " + std::to_string(model.averagePayoff) + "\n";
    output += "          Standard Error: " + std::to_string(model.standardError) + "\n";

    double confidenceMargin = 1.96 * model.standardError;
    double lowerBound = model.averagePayoff - confidenceMargin;
    double upperBound = model.averagePayoff + confidenceMargin;

    output += "95% probability interval: (" + std::to_string(lowerBound) + ", " + std::to_string(upperBound) + ")\n";
    output += "Greeks:\n";
    output += "                   Delta: " + std::to_string(model.greeks.delta) + "\n";
    output += "                   Gamma: " + std::to_string(model.greeks.gamma) + "\n";
    output += "                    Vega: " + std::to_string(model.greeks.vega) + "\n";
    output += "                     Rho: " + std::to_string(model.greeks.rho) + "\n";
    output += "                   Theta: " + std::to_string(model.greeks.theta) + "\n";

    return output;

}

int main() {

    OptionParams callOption { 100, 100, 1, 0.05, 0.2, OptionType::Call };
    OptionParams putOption { 100, 100, 1, 0.05, 0.2, OptionType::Put };
    OptionModel callModel = monteCarloModel(callOption); // Expecting ~38
    std::cout << std::endl << modelToString("Call option", callModel);
    OptionModel putModel = monteCarloModel(putOption); // Expecting ~3
    std::cout << modelToString("Put option", putModel);
    return 0;

}
