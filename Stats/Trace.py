import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sb

class Trace:

    def __init__(self, traceName: str, dataframe: pd.DataFrame):
        self.traceName = traceName
        self.dataframe = dataframe[dataframe["traceName"] == traceName]

    def plotCachePerformance(self):
        # self.dataframe.plot.hexbin("numIndexBits", "numWays", "cacheMissRate")
        # plt.show()
        tabla = self.dataframe.pivot_table( "cacheMissRate","numIndexBits", "numWays")
        sb.heatmap(tabla, annot=True, fmt=".4f")
        plt.show()

    def plotDictionaryPerformance(self):
        # self.dataframe.plot.scatter("numClasses", "dictionaryMissRate")
        # plt.scatter(self.dataframe.numClasses, self.dataframe.dictionaryMissRate)
        # plt.show()
        tabla = self.dataframe.pivot_table("dictionaryMissRate", "cacheMissRate", "numClasses")
        sb.heatmap(tabla, annot=True, fmt=".4f")
        plt.show()
        pass

    def plotPredictorModelPerformance(self):
        dataframe = pd.DataFrame(self.dataframe)
        dataframe['modelHitRate'] = dataframe['hitRate'] / ((1 - dataframe['cacheMissRate']) * (1 - dataframe['dictionaryMissRate']))
        tabla = dataframe.pivot_table("modelHitRate", "numClasses", "numSequenceAccesses")
        sb.heatmap(tabla, annot=True, fmt=".4f")
        plt.show()
        pass
