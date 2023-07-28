import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sb
import numpy as np
from matplotlib import ticker
import seaborn as sns
from PredictorType import metricTranslationTable, PredictorsHelper, possiblePredictorTypes, defaultOrderLevels

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
        '''
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
        '''
        self.predictorsManager = PredictorsHelper(possiblePredictorTypes, defaultOrderLevels)
        self.predictorsManager.setPredictors( self.dataframe.to_dict("list"))
        # self.dataframe = self.predictorsManager.setPredictorsNameAndTranslatedToDataframe(self.dataframe)
        
    def groupAndAggregate(self, inputParameters: list, outputParameters: list):
        map = {param: ['mean', 'std'] for param in outputParameters}
        return self.dataframe.groupby(inputParameters).agg(map)

    def plotPerformanceComparison(self, metric, includeMean=True):
        if self.dataframe[metric].min() > 0.001:
            dataframe = self.dataframe.round(3)
        else:
            dataframe = self.dataframe
        dataframe['predictorHitRate'] = dataframe['hitRate']
        dataframe['order'] = dataframe['predictorPrettyName']
        dataframe = dataframe.replace({"order":
               self.predictorsManager.getPredictorsOrder(self.predictorsManager.predictors)})
        dataframe = dataframe.sort_values(by=['order'])
        metrics = []
        # Group the DataFrame by 'Category'
        groupedByTrace = dataframe.groupby('traceName', sort=False)
        groupedByPredictor = dataframe.groupby('predictorPrettyName', sort=False)

        # Create a figure and axis
        # fig, ax = plt.subplots(layout='constrained')
        fig, ax = plt.subplots(layout='constrained')

        x = np.arange(len(groupedByTrace))  # the label locations
        # width = 0.15  # the width of the bars
        width = 0.07  # the width of the bars
        multiplier = 0

        # Iterate over each group and plot the bars
        # for name, group in grouped:
        labels = []
        for group in groupedByPredictor:
            offset = width * multiplier
            g = group[1].sort_values(by=['traceName'])
            # labels.append(predictorTypeTranslationTable[group[0]])
            labels.append(group[0])
            rects = ax.bar(x + offset,
                           g[metric],
                           width,
                           # label=predictorTypeTranslationTable[group[0]])
                           label=group[0])
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
            ncol=3, fontsize=8)

        '''
        if plotMemoryCosts:
            ax.get_yaxis().set_major_formatter(
                ticker.FuncFormatter(lambda x, p: '{:.2e}'.format(x)))
        '''

        # Show the plot
        plt.show()

        plt.figure()
        tuples = [(p[0],
                   self.predictorsManager.getPredictorsOrder(self.predictorsManager.predictors)[p[0]]) for p in groupedByPredictor]
        order = list(
            sorted(tuples, key=lambda t: t[1])
        )
        order = list(map(lambda t: t[0], order))
        sns.boxplot(x='predictorPrettyName', y=metric, data=self.dataframe,
                    order=order,
                    #[
                    #    'InfiniteDFCM', 'InfiniteDFCMGradeK', 'InfiniteBufferSVM', 'RealBufferSVM',
                    #    'RealBufferSVM_4_4', 'RealBufferSVM_8_8',
                    #],
                    showmeans=includeMean,
                    meanprops={'marker': 'o',
                               'markerfacecolor': 'white',
                               'markeredgecolor': 'black',
                               'markersize': '8'}
                    ).set(xlabel=None, ylabel=None)
        plt.grid(True, axis='y', linestyle='--', alpha=0.4)
        plt.locator_params(axis='y', nbins=20)

        # plt.xticks([0, 1, 2, 3], labels,
        plt.xticks(list(range(0, len(labels))), labels,
                   rotation=15
                   )
        plt.show()

    # https://sirinnes.wordpress.com/2013/04/25/pareto-frontier-graphic-via-python/

    def __getParettoFront(self, values1, values2, max1, max2):
        if len(values1) <= 0 or len(values2) <=0:
            return [], []
        sorted_list = sorted([[values1[i], values2[i]] for i in range(len(values1))], reverse=max1)
        sorted_list_indexes = sorted([i for i in range(len(values1))], key = lambda i: [values1[i], values2[i]], reverse=max1)
        pareto_front = [sorted_list[0]]
        pareto_front_nodes = set()
        pareto_front_nodes.add(sorted_list_indexes[0])
        i = 1
        for pair in sorted_list[1:]:
            if max2:
                if pair[1] >= pareto_front[-1][1]:
                    pareto_front.append(pair)
                    pareto_front_nodes.add(sorted_list_indexes[i])
            else:
                if pair[1] <= pareto_front[-1][1]:
                    pareto_front.append(pair)
                    pareto_front_nodes.add(sorted_list_indexes[i])
            i = i + 1
        return pareto_front_nodes, pareto_front

    def plotParettoFront(self, metric1, metric2, max1: bool, max2: bool, plotAllFronts = False):
        group = self.groupAndAggregate(['predictorPrettyName'], [metric1, metric2])
        values1 = group[(metric1, 'mean')].to_list()
        values2 = group[(metric2, 'mean')].to_list()
        labels = group.index.to_list()


        '''Plotting process'''
        # plt.scatter(values1, values2)
        fig, ax = plt.subplots()

        for i in range(0, len(labels)):
            ax.scatter(values1[i], values2[i], label=labels[i], s=50)

        '''Pareto frontier selection process'''
        all_pareto_front_nodes = []
        pareto_back_nodes = [i for i in range(0, len(values1)) if not i in all_pareto_front_nodes]
        pareto_fronts = []
        while len(pareto_back_nodes) > 0:
            values1_ = [values1[i] for i in pareto_back_nodes]
            values2_ = [values2[i] for i in pareto_back_nodes]
            pareto_front_nodes, pareto_front = self.__getParettoFront(values1_, values2_, max1, max2)
            pareto_front_nodes = [pareto_back_nodes[k] for k in pareto_front_nodes]
            all_pareto_front_nodes.extend(pareto_front_nodes)
            pareto_back_nodes = [i for i in range(0, len(values1)) if not i in all_pareto_front_nodes]
            pareto_fronts.append(pareto_front)
            if not plotAllFronts:
                break

        for pareto_f in pareto_fronts:
            pf_X = [pair[0] for pair in pareto_f]
            pf_Y = [pair[1] for pair in pareto_f]
            plt.plot(pf_X, pf_Y)

        plt.xlabel(metricTranslationTable[metric1])
        plt.ylabel(metricTranslationTable[metric2])
        plt.legend(  # bbox_to_anchor=(0.75, 1.15),
            bbox_to_anchor=(0., 1.02, 1., .102),
            ncol=3, fontsize=8)
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


        # OJO: Ajustar coste en memoria a capacidad para los DFCMs:

        casosDFCMHashOnHash = self.dataframe['predictorType'] == "InfiniteDFCM"
        casosDFCM8Order = self.dataframe['predictorType'] == "InfiniteDFCMGradeK"
        dataframe_ = self.dataframe.copy()
        dataframe_['firstTableMemoryCost'] = \
            self.dataframe[casosDFCMHashOnHash]['firstTableMemoryCost'] * 2 / 3
        dataframe_['secondTableMemoryCost'] = \
            self.dataframe[casosDFCMHashOnHash]['secondTableMemoryCost'] * 1 / 2
        dataframe_['totalMemoryCost'] = dataframe_['firstTableMemoryCost'] + dataframe_['secondTableMemoryCost']
        if casosDFCMHashOnHash.any():
            self.dataframe[casosDFCMHashOnHash] = dataframe_[casosDFCMHashOnHash]


        dataframe_ = self.dataframe.copy()
        dataframe_['firstTableMemoryCost'] = \
            self.dataframe[casosDFCM8Order]['firstTableMemoryCost'] * (1 + 8) / (2 + 8)
        dataframe_['secondTableMemoryCost'] = \
            self.dataframe[casosDFCM8Order]['secondTableMemoryCost'] * 1 / 2
        dataframe_['totalMemoryCost'] = dataframe_['firstTableMemoryCost'] + dataframe_['secondTableMemoryCost']
        if casosDFCM8Order.any():
            self.dataframe[casosDFCM8Order] = dataframe_[casosDFCM8Order]


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
        ax.set_ylabel("Total mean memory capacity (bytes)")
        # if title != None:
        #    ax.set_title(title)
        ax.set_xticks(x, grouped.groups.keys())

        # Show the plot
        plt.show()

    def plotPerformanceByTraceName(self, plotMemoryCosts = False, title = None):
        dataframe = self.dataframe.round(3)
        dataframe['predictorHitRate'] = dataframe['hitRate']
        metrics = []
        if "BufferSVM" in self.predictorType:
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
            plt.ylabel("Mean memory capacity (bytes)")

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
