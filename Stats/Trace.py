import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sb
import numpy as np
from matplotlib import ticker
import seaborn as sns


metricTranslationTable = {
    'hitRate': 'Predictor hit rate',
    'totalMemoryCost': 'Total memory cost',
    'cacheMemoryCost': 'Cache memory cost',
    'cacheMissRate': 'Cache miss rate',
    'cacheHitRate': 'Input buffer hit rate',
    'dictionaryMemoryCost': 'Dictionary memory cost',
    'dictionaryMissRate': 'Dictionary miss rate',
    'dictionaryHitRate': 'Dictionary hit rate',
    'firstTableMissRate': 'First table miss rate',
    'firstTableHitRate': 'First table hit rate',
    'secondTableMissRate': 'Second table miss rate',
    'secondTableHitRate': 'Second table hit rate',
    'firstTableMemoryCost': 'First table memory cost',
    'secondTableMemoryCost': 'Second table memory cost',
    'modelMemoryCost': 'Model memory cost',
    'modelHitRate': 'Model hit rate',
    'predictorHitRate': 'Predictor hit rate',
}

predictorTypeTranslationTable = {
    'InfiniteDFCM': 'Ideal HashOnHash DFCM',
    'InfiniteBufferSVM': 'Ideal SVM4AP',
    'RealBufferSVM': 'Real SVM4AP',
    'InfiniteDFCMGradeK': 'Ideal 8-order DFCM'
}

predictorTypeOrder = {
    'InfiniteDFCM': 0,
    'InfiniteBufferSVM': 2,
    'RealBufferSVM': 3,
    'InfiniteDFCMGradeK': 1
}

class ExploratoryTrace:

    def __init__(self, traceName: str, dataframe: pd.DataFrame):
        self.traceName = traceName
        self.dataframe = dataframe[dataframe["traceName"] == traceName]
        self.dataframe['modelHitRate'] = \
            self.dataframe['hitRate'] / ((1 - self.dataframe['cacheMissRate']) * (1 - self.dataframe['dictionaryMissRate']))
        self.dataframe['buffersHitRate'] = \
            (1 - self.dataframe['cacheMissRate']) * (1 - self.dataframe['dictionaryMissRate'])

    def groupAndAggregate(self, inputParameters: list, outputParameters: list):
        map = {param: ['mean', 'std'] for param in outputParameters}
        return self.dataframe.groupby(inputParameters).agg(map)

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
        tabla = dataframe.pivot_table("modelHitRate", "numClasses", "numSequenceAccesses")
        sb.heatmap(tabla, annot=True, fmt=".4f")
        plt.show()
        pass

class TraceComparer:
    def __init__(self, dataframe):
        self.dataframe = dataframe
        self.dataframe['modelHitRate'] = \
            self.dataframe['hitRate'] / (
                    (1 - self.dataframe['cacheMissRate']) * (1 - self.dataframe['dictionaryMissRate']))
        self.dataframe['cacheHitRate'] = \
            (1 - self.dataframe['cacheMissRate'])
        self.dataframe['dictionaryHitRate'] = \
            (1 - self.dataframe['dictionaryMissRate'])
        self.dataframe['firstTableHitRate'] = \
            (1 - self.dataframe['firstTableMissRate'])
        self.dataframe['secondTableHitRate'] = \
            (1 - self.dataframe['secondTableMissRate'])

    def groupAndAggregate(self, inputParameters: list, outputParameters: list):
        map = {param: ['mean', 'std'] for param in outputParameters}
        return self.dataframe.groupby(inputParameters).agg(map)

    def plotPerformanceComparison(self, metric, includeMean=True):
        dataframe = self.dataframe.round(3)
        dataframe['predictorHitRate'] = dataframe['hitRate']
        dataframe['order'] = dataframe['predictorType']
        dataframe = dataframe.replace({"order": predictorTypeOrder})
        dataframe = dataframe.sort_values(by=['order'])
        metrics = []
        # Group the DataFrame by 'Category'
        groupedByTrace = dataframe.groupby('traceName', sort=False)
        groupedByPredictor = dataframe.groupby('predictorType', sort=False)

        # Create a figure and axis
        # fig, ax = plt.subplots(layout='constrained')
        fig, ax = plt.subplots(layout='constrained')

        x = np.arange(len(groupedByTrace))  # the label locations
        width = 0.2  # the width of the bars
        multiplier = 0

        # Iterate over each group and plot the bars
        # for name, group in grouped:
        labels = []
        for group in groupedByPredictor:
            offset = width * multiplier
            g = group[1].sort_values(by=['traceName'])
            labels.append(predictorTypeTranslationTable[group[0]])
            rects = ax.bar(x + offset,
                           g[metric],
                           width, label=predictorTypeTranslationTable[group[0]])
            '''
            if plotMemoryCosts:
                ax.bar_label(rects, padding=5, fontsize=6, rotation=30,
                             fmt = '%.1e')
            else:
                ax.bar_label(rects, padding=5, fontsize=6, rotation=30,
                             # fmt = '%.2e')
                             fmt='%.3f')
            '''
            multiplier += 1

        '''
        if includeMean:
            aggregateResults = self.groupAndAggregate(['predictorType'], [metric])
            # group = aggregateResults[(metric, 'mean')].sort_index(key=lambda index:
            #    index.sort_values(key=lambda s: predictorTypeOrder[s])[0])
            group = aggregateResults[(metric, 'mean')]
            indexes = group.values.tolist()
            labels.append('Mean')
            rects = ax.bar(x + offset,
                           group,
                           width, label=predictorTypeTranslationTable[group[0]])
        '''
        # Add some text for labels, title and custom x-axis tick labels, etc.
        # ax.set_ylabel('Metric value')
        # if title != None:
        #    ax.set_title(title)
        ax.set_xticks(x + width, sorted(groupedByTrace.groups.keys()))
        #if plotMemoryCosts:
        #    plt.ylabel("Memory cost (bytes)")

        plt.yticks(fontsize=9)
        plt.xticks(fontsize=10, rotation=30)
        # ax.legend(loc='upper left', fontsize=10, )
        plt.locator_params(axis='y', nbins=20)
        plt.grid(True, axis='y', linestyle='--')

        plt.legend(  # bbox_to_anchor=(0.75, 1.15),
            bbox_to_anchor=(0., 1.02, 1., .102),
            ncol=len(groupedByPredictor), fontsize=8)

        '''
        if plotMemoryCosts:
            ax.get_yaxis().set_major_formatter(
                ticker.FuncFormatter(lambda x, p: '{:.2e}'.format(x)))
        '''

        # Show the plot
        plt.show()

        plt.figure()

        sns.boxplot(x='predictorType', y='hitRate', data=self.dataframe,
                    order=['InfiniteDFCM', 'InfiniteDFCMGradeK', 'InfiniteBufferSVM', 'RealBufferSVM'],
                    showmeans=includeMean,
                    meanprops={'marker': 'o',
                               'markerfacecolor': 'white',
                               'markeredgecolor': 'black',
                               'markersize': '8'}
                    ).set(xlabel=None, ylabel=None)
        plt.grid(True, axis='y', linestyle='--', alpha=0.4)
        plt.locator_params(axis='y', nbins=20)
        plt.xticks([0, 1, 2, 3], labels,
                   # rotation=30
                   )
        plt.show()


class WholeTrace:

    def __init__(self, predictorType: str, dataframe: pd.DataFrame):
        self.predictorType = predictorType
        self.dataframe = dataframe[dataframe["predictorType"] == predictorType]
        self.dataframe['modelHitRate'] = \
            self.dataframe['hitRate'] / (
                        (1 - self.dataframe['cacheMissRate']) * (1 - self.dataframe['dictionaryMissRate']))
        self.dataframe['cacheHitRate'] = \
            (1 - self.dataframe['cacheMissRate'])
        self.dataframe['dictionaryHitRate'] = \
            (1 - self.dataframe['dictionaryMissRate'])
        self.dataframe['firstTableHitRate'] = \
            (1 - self.dataframe['firstTableMissRate'])
        self.dataframe['secondTableHitRate'] = \
            (1 - self.dataframe['secondTableMissRate'])

    def groupAndAggregate(self, inputParameters: list, outputParameters: list):
        map = {param: ['mean', 'std'] for param in outputParameters}
        return self.dataframe.groupby(inputParameters).agg(map)

    def plotOneMetricByTraceName(self, metric, title=None):
        dataframe = self.dataframe.round(3)
        dataframe['predictorHitRate'] = dataframe['hitRate']
        grouped = dataframe.groupby('traceName')
        fig, ax = plt.subplots(layout='constrained')

        x = np.arange(len(grouped))  # the label locations
        width = 0.7  # the width of the bars
        multiplier = 0

        # Iterate over each group and plot the bars
        # for name, group in grouped:
        offset = width * multiplier
        rects = ax.bar(x + offset, dataframe[metric], width, label=metricTranslationTable[metric])
        # ax.bar_label(rects, padding=0, fontsize=9)
        multiplier += 1

        plt.yticks(fontsize=9)
        plt.xticks(fontsize=10, rotation=30)
        # ax.legend(loc='upper left', fontsize=10, )
        plt.locator_params(axis='y', nbins=20)
        plt.grid(True, axis='y', linestyle='--')

        # Add some text for labels, title and custom x-axis tick labels, etc.
        ax.set_ylabel(metricTranslationTable[metric])
        ax.set_ylabel("Total memory cost (bytes)")
        # if title != None:
        #    ax.set_title(title)
        ax.set_xticks(x, grouped.groups.keys())

        # Show the plot
        plt.show()

    def plotPerformanceByTraceName(self, plotMemoryCosts = False, title = None):
        dataframe = self.dataframe.round(3)
        dataframe['predictorHitRate'] = dataframe['hitRate']
        metrics = []
        if self.predictorType == "InfiniteBufferSVM" or self.predictorType == "RealBufferSVM":
            if plotMemoryCosts:
                # metrics = ["cacheMemoryCost", "dictionaryMemoryCost", "modelMemoryCost"]
                metric = "totalMemoryCost"
                self.plotOneMetricByTraceName(metric, title)
                return
            else:
                metrics = ["cacheHitRate", "dictionaryHitRate",
                           "modelHitRate", "predictorHitRate"]
        elif self.predictorType == "InfiniteDFCM" or self.predictorType == "InfiniteDFCMGradeK":
            if plotMemoryCosts:
                metrics = ["firstTableMemoryCost", "secondTableMemoryCost", "totalMemoryCost"]
            else:
                metrics = ["firstTableHitRate", "secondTableHitRate",
                           "predictorHitRate"]
        else:
            raise Exception("Unknown predictor type")

        # Group the DataFrame by 'Category'
        grouped = dataframe.groupby('traceName')

        # Create a figure and axis
        # fig, ax = plt.subplots(layout='constrained')
        fig, ax = plt.subplots(layout='constrained')

        x = np.arange(len(grouped))  # the label locations
        width = 0.2  # the width of the bars
        multiplier = 0

        # Iterate over each group and plot the bars
        # for name, group in grouped:
        for metric in metrics:
            offset = width * multiplier
            rects = ax.bar(x + offset, dataframe[metric], width, label=metricTranslationTable[metric])
            '''
            if plotMemoryCosts:
                ax.bar_label(rects, padding=5, fontsize=6, rotation=30,
                             fmt = '%.1e')
            else:
                ax.bar_label(rects, padding=5, fontsize=6, rotation=30,
                             # fmt = '%.2e')
                             fmt='%.3f')
            '''
            multiplier += 1

        # Add some text for labels, title and custom x-axis tick labels, etc.
        # ax.set_ylabel('Metric value')
        # if title != None:
        #    ax.set_title(title)
        ax.set_xticks(x + width, grouped.groups.keys())
        if plotMemoryCosts:
            plt.ylabel("Memory cost (bytes)")

        plt.yticks(fontsize=9)
        plt.xticks(fontsize=10, rotation=30)
        # ax.legend(loc='upper left', fontsize=10, )
        plt.locator_params(axis='y', nbins=20)
        plt.grid(True, axis='y', linestyle='--')

        plt.legend(# bbox_to_anchor=(0.75, 1.15),
                bbox_to_anchor=(0., 1.02, 1., .102),
                ncol=len(metrics), fontsize=8)

        '''
        if plotMemoryCosts:
            ax.get_yaxis().set_major_formatter(
                ticker.FuncFormatter(lambda x, p: '{:.2e}'.format(x)))
        '''

        # Show the plot
        plt.show()
