#include <iostream>
#include "MonteCarlo.h"

int main() {
    OptionParams amazonOption { 226.13, 235, 1.164, 0.044, 0.2866, OptionType::Call };
    OptionResult amazonModel = runMonteCarloSimulation(amazonOption);
    std::cout << amazonModel.averagePayoff << std::endl;
    std::cout << amazonModel.standardError << std::endl;
    std::cout << amazonModel.greeks.delta << std::endl;
    std::cout << amazonModel.greeks.gamma << std::endl;
    std::cout << amazonModel.greeks.vega << std::endl;
    std::cout << amazonModel.greeks.rho << std::endl;
    std::cout << amazonModel.greeks.theta << std::endl;

    return 0;
}