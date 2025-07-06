#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>

struct optionParams {
    double S0; // Spot price
    double K; // Strike/Exercise price
    double T; // Time to expiry
    double r; // Risk-free interest rate
    double sigma; // Volatility
    int n; // Number of simulations
    int m; // Number of time steps
};

double monteCarloCallPrice(optionParams params) {
    assert(("Number of simulations must be positive.", params.n > 0));
    assert(("Number of time steps must be positive.", params.m > 0));

    double payoffSum = 0;
    double dt = params.T / params.m;

    // Initialising our random variable generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(0.0, 1.0);

    for (int i = 0; i < params.n; i++) {
        double S = params.S0;

        for (int j = 0; j < params.m; j++) {
            double Z = distribution(generator); // Z ~ N(0,1)
            S *= exp((params.r - (pow(params.sigma, 2) / 2)) * dt + params.sigma * sqrt(dt) * Z);
        }

        double payoff = std::max(S - params.K, 0.0);
        payoffSum += payoff;
    }

    double averagePayoff = payoffSum / params.n;
    return exp(-params.r * params.T) * averagePayoff;
}

int main() {
    optionParams example;
    example.S0 = 140; example.K = 110; example.T = 1; example.r = 0.05;
    example.sigma = 0.3; example.n = 10000; example.m = 252;
    std::cout << monteCarloCallPrice(example); // expecting ~38
    return 0;
}
