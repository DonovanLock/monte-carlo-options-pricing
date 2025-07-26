#include <iostream>
#include <string>
#include "MonteCarlo.h"
#include <Utils.h>

int main() {
    OptionParams amazonOption { 226.13, 235, 1.164, 0.044, 0.2866, OptionType::Call };
    OptionResult amazonModel = runMonteCarloSimulation(amazonOption);
    outputResults(amazonModel);

    return 0;
}