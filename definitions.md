# Definitions
## Options
Options are a type of financial derivative, i.e. a contract between a buyer and seller. By using
options, investors may speculate on the price of a given security (a tradeable asset) at a later
moment in time known as the expiration date, without being obligated buy or sell this option. All
options are either call options or put options.

During a call option, a buyer pays a premium to gain the option to buy the underlying security at a
pre-determined price, agreed with the seller. This is known as either the strike price or the exercise
price. If the security's price is above the strike price by the expiration date, the buyer may wish to
exercise their option by purchasing the security. Hence, they now own a stock for the value of the
strike price minus the premium. So if strike price + premium < spot price, they have made a profit.
Alternatively, if the security has decreased in price, then the buyer may choose not to buy it, as
they have no such obligation. This results in them losing the money paid for the premium. Of course,
any money made by the buyer is lost by the seller, and vice versa.

A put option is extremely similar to a call option, save that the buyer gains the right, but not
obligation, to sell the underlying security. This right is given after paying the premium, as with
call options. So buyers of put options make a profit if the security's value sufficiently decreases
below the the strike price.

With European options, the holder may only exercise their option on the expiration date. American
options instead allow the holder to exercise their option at any point between the purchase date and
the expiration date. As American options give the holder an increased degree of flexibility, they
generally have higher premiums than European options.

## Monte Carlo modelling
Due to the uncertain – and potentially lucrative – nature of options, pricing them is an extremely
useful skill. This is achieved through options pricing models, which are predictive algorithms. One
such example is with Monte Carlo modelling, which simulates the security's unpredictability with many
random simulations before taking an average of these results.

Specifically, we use Geometric Brownian Motion to replicate the drift and randomness of the underlying
security. This requires a number of variables, such as the risk-free interest rate (denoted by 'r') or
the the security's volatility (denoted by the Greek letter 'sigma'). The risk-free interest rate is
the hypothetical rate of return on a completely safe investment; this is used as a benchmark for
further investments, and is typically defined based on stable, government-issued securities.
Volatility is a quantitative measurement of a security's uncertainty, i.e. the rate and strengths of
its fluctuations over time.

As Monte Carlo modelling uses a large number of simulations, we are also able to apply the Central
Limit Theorem in order to give us a confidence bound. Specifically, this program outputs not just an
estimate for the option's value, but also an interval which has roughly a 95% probability of
containing the option's true price.

## Risk analysis
To quantify how sensitive an option is to different factors, we use partial derivatives to analyse
how sensitive its price is to certain parameters such as volatility or the spot price. These values
are called the Greeks, and this program outputs the five Greeks that are most useful for options
pricing:

- Delta: the derivative of the option value with respect to the spot price.
- Gamma: the second derivative of the option value with respect to the spot price.
- Vega: the derivative of the option value with respect to the volatility.
- Rho: the derivative of the option value with respect to the risk-free interest rate.
- Theta: the derivative of the option value with respect to the time to expiry, multiplied by -1.

## Variance reduction
Due to the use of random variables, a degree of variance is unavoidable in Monte Carlo modelling.
However, we can minimise this variance with variance reduction techniques. One example is the
antithetic variates method, wherein we construct paths with negative covariance and take their average
payoff. This minimises variance due to the formula for the variance of a sum of two random variables. 