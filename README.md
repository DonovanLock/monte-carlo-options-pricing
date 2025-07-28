# monte-carlo-options-pricing
## Description
This program uses Monte Carlo modelling methods to price European options. To price an option, the following parameters are required:
- Spot price
- Strike price
- Time to expiry
- Risk-free interest rate
- Volatility
- Option type (call or sell)

The program then generates many price paths with Geometric Brownian motion, before returning an estimated price and a graph of these paths.

![Graph of many simulated price paths over time](/exampleGraph.png)

It also calculates some relevant Greeks (delta, gamma, vega, rho, and theta), and uses the antithetic
variates method to reduce variance.

For an overview of the concepts, methodology, and mathematics involved, click [here](definitions.md).

## Installation
To run this program, first clone and enter the repository:
```
git clone https://github.com/DonovanLock/monte-carlo-options-pricing.git
cd monte-carlo-options-pricing
```

Then simply run `MonteCarlo.exe`. For information on arguments, run `MonteCarlo.exe -h`.
