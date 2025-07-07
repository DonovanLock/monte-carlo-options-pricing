#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

enum class OptionType { Call, Put };

struct optionParams {

    double S0; // Spot price
    double K; // Strike/Exercise price
    double T; // Time to expiry
    double r; // Risk-free interest rate
    double sigma; // Volatility
    OptionType optiontype; // Call or Sell

};

struct optionModel {

    double averagePayoff;
    double standardError;

};

const int n = 100000; // Number of simulations

optionModel monteCarloModel(optionParams params) {

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

}

std::string modelToString(std::string name, optionModel model) {

    std::string output = name + ", modelled with " + std::to_string(n) + " simulations:\n";
    output += "  Estimated Option Price: " + std::to_string(model.averagePayoff) + "\n";
    output += "          Standard Error: " + std::to_string(model.standardError) + "\n";
    double confidenceMargin = 1.96 * model.standardError;
    double lowerBound = model.averagePayoff - confidenceMargin;
    double upperBound = model.averagePayoff + confidenceMargin;
    output += "95% probability interval: (" + std::to_string(lowerBound) + ", " + std::to_string(upperBound) + ")\n\n";
    return output;

}

int main() {

    optionParams callOption { 140, 110, 1, 0.05, 0.3, OptionType::Call };
    optionParams putOption { 140, 110, 1, 0.05, 0.3, OptionType::Put };
    optionModel callModel = monteCarloModel(callOption); // Expecting ~38
    optionModel putModel = monteCarloModel(putOption); // Expecting ~3
    std::cout << std::endl << modelToString("Call option", callModel);
    std::cout << modelToString("Put option", putModel);
    return 0;

}
