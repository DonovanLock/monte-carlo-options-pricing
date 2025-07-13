import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

workingDirectory = Path(__file__).parent
dataFileName = workingDirectory / "graphData.csv"
data = np.loadtxt(dataFileName, delimiter=",")
timeSteps = data.shape[1]
time = np.arange(timeSteps)

for path in data:
    plt.plot(time, path, alpha=0.3)

plt.xlabel("Days")
plt.ylabel("Simulated Security Price")
plt.title("Monte Carlo Simulated Price Paths")
plt.show()