#pragma once

enum class OptionType { Call, Put };

struct OptionParams {
    double spotPrice;
    double strikePrice;
    double timeToMaturity; // Measured in years
    double riskFreeRate;
    double volatility;
    OptionType optionType;
};

struct Greeks {
    double delta;
    double gamma;
    double vega;
    double rho;
    double theta;
};

struct OptionResult {
    double averagePayoff;
    double standardError;
    Greeks greeks;
};
