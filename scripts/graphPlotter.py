import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

rootDirectory = Path(__file__).parent.parent
dataFileName = rootDirectory / "output" / "graphData.csv"
data = np.loadtxt(dataFileName, delimiter=",")
timeSteps = data.shape[1]
time = np.arange(timeSteps)

for path in data:
    plt.plot(time, path, alpha=0.3)

plt.gca().set_xlim([0,timeSteps-1])
plt.xlabel("Days")
plt.ylabel("Simulated Security Price")
plt.title("Monte Carlo Simulated Price Paths")
graphFileName = rootDirectory / "output" / "graph.png"
plt.savefig(graphFileName, bbox_inches="tight")
plt.show()