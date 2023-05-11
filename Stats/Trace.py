import pandas as pd
import matplotlib.pyplot as plt

class Trace:

    def __init__(self, traceName: str, dataframe: pd.DataFrame):
        self.traceName = traceName
        self.dataframe = dataframe[dataframe["traceName"] == traceName]

    def plotCachePerformance(self):
        self.dataframe.plot.hexbin("numIndexBits", "numWays", "cacheMissRate")
        plt.show()

    def plotDictionaryPerformance(self):
        self.dataframe.plot.scatter("numClasses", "dictionaryMissRate")
        # plt.scatter(self.dataframe.numClasses, self.dataframe.dictionaryMissRate)
        plt.show()
