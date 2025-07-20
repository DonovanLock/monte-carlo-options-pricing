# monte-carlo-options-pricing
This program uses Monte Carlo modelling methods to price European options. To price an option, the 
following parameters are required:
- Spot price
- Strike price
- Time to expiry
- Volatility
- Option type (call or sell)

The program then generates many price paths with Geometric Brownian motion, in order to approximate
the option's price. It also returns a graph of these paths and calculates some relevant Greeks (delta, gamma, vega, rho, and theta).

For an explanation of the methodology or mathematics involved, click [here](definitions.md).