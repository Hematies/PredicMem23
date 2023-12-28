import math

import numpy as np
import pandas as pd

from PredictorType import PredictorsHelper, defaultOrderLevels, possiblePredictorTypes
from XMLFile import XMLFile
import os

class MultiXMLReader:

    def __init__(self, directoryPath, onlyPredictorsOfTypes = []):
        self.directoryPath = directoryPath
        self.files = self.getXMLFiles(directoryPath)
        self.dataframe = self.buildDataframe(self.files)
        self.dataframe['dictionaryMissRate'] = \
            self.dataframe['dictionaryMissRate'] - self.dataframe['cacheMissRate'] # FIX
        self.dataframe['modelHitRate'] = \
            self.dataframe['hitRate'] / (
                        (1 - self.dataframe['cacheMissRate']) * (1 - self.dataframe['dictionaryMissRate']))

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

        self.predictorsManager = PredictorsHelper(possiblePredictorTypes, defaultOrderLevels)
        self.predictorsManager.setPredictors(self.dataframe.to_dict("list"))
        self.dataframe = self.predictorsManager.setPredictorsNameAndTranslatedToDataframe(self.dataframe)
        self.dataframe = self.predictorsManager.setPredictorsMemoryCostsToDataframe(self.dataframe)
        self.dataframe["yield"] = self.dataframe["hitRate"] / self.dataframe["totalMemoryCost"]

        if len(onlyPredictorsOfTypes) > 0:
            # originalDataframe = pd.DataFrame(self.dataframe)
            mascara = np.zeros(len(self.dataframe))
            for predictorType in onlyPredictorsOfTypes:
                mascara = np.logical_or(mascara, self.dataframe['predictorType'] == predictorType)
            self.dataframe = self.dataframe[mascara]

        self.dataframe = self.__computeTotalMemoryCost(self.dataframe)

    def __computeTotalMemoryCost(self, dataframe):
        # res = pd.DataFrame(dataframe)
        res = dataframe.to_dict()
        wordSize = 64
        lruBits = 1
        for index in dataframe.index:
            type = dataframe['predictorType'][index]
            if not "Infinite" in type:
                if "DFCM" in type:
                    firstTableIndexBits = dataframe["firstTableNumIndexBits"][index]
                    firstTableNumWays = dataframe["firstTableNumWays"][index]
                    secondTableIndexBits = dataframe["secondTableNumIndexBits"][index]
                    secondTableNumWays = dataframe["secondTableNumWays"][index]
                    numSequenceAccesses = dataframe["numSequenceAccesses"][index]

                    if "HoH" in type:
                        firstTableEntryCost = \
                            (wordSize - firstTableIndexBits) + wordSize * 2 + lruBits
                    else:
                        firstTableEntryCost = \
                            (wordSize - firstTableIndexBits) + wordSize * (1 + numSequenceAccesses) + lruBits
                    secondTableEntryCost = \
                        (wordSize - secondTableIndexBits) + wordSize + lruBits

                    firstTableCost = firstTableEntryCost / 8 * (2**firstTableIndexBits) * firstTableNumWays
                    # res.loc["firstTableMemoryCost", index] = firstTableCost
                    res["firstTableMemoryCost"][index] = firstTableCost
                    secondTableCost = secondTableEntryCost / 8 * (2 ** secondTableIndexBits) * secondTableNumWays
                    # res.loc["secondTableMemoryCost", index] = secondTableCost
                    res["secondTableMemoryCost"][index] = secondTableCost
                    totalCost = firstTableCost + secondTableCost
                    # res.loc["totalMemoryCost", index] = totalCost
                    res["totalMemoryCost"][index] = totalCost
                else:
                    cacheIndexBits = dataframe["numIndexBits"][index]
                    cacheNumWays = dataframe["numWays"][index]
                    numSequenceAccesses = dataframe["numSequenceAccesses"][index]
                    numClasses = dataframe["numClasses"][index]
                    maxConfidence = dataframe["maxConfidence"][index] + 1

                    numClassBits =math.ceil(math.log2(numClasses + 1))
                    tableEntryCost = (wordSize - cacheIndexBits) + wordSize + \
                        (numSequenceAccesses * numClassBits) + lruBits
                    tableCost = tableEntryCost / 8.0  * (2 ** cacheIndexBits) * cacheNumWays

                    dictionaryEntryCost = math.ceil(math.log2(numClasses)) + math.ceil(math.log2(maxConfidence)) + wordSize
                    dictionaryCost = dictionaryEntryCost / 8.0 * numClasses

                    modelCost = dataframe["modelMemoryCost"][index]

                    totalCost = tableCost + dictionaryCost + modelCost
                    # res.at["cacheMemoryCost", index] = tableCost
                    res["cacheMemoryCost"][index] = tableCost
                    # res.at["dictionaryMemoryCost", index] = dictionaryCost
                    res["dictionaryMemoryCost"][index] = dictionaryCost
                    # res.at["totalMemoryCost", index] = totalCost
                    res["totalMemoryCost"][index] = totalCost
        res = pd.DataFrame.from_dict(res)
        return res
    def getXMLFiles(self, directoryPath):
        res = []
        allFiles = os.listdir(directoryPath)
        for file in allFiles:
            path = os.path.join(directoryPath, file)
            if os.path.isfile(path):
                extension = os.path.splitext(file)[1]
                if extension == ".xml" or extension == "xml":
                    f = XMLFile(path)
                    res.append(f)
        return res

    def buildDataframe(self, files):
        dataframes = []
        i = 0
        for file in files:
            dataframes.append(file.getTraceLevelDataframe())
            i = i + 1
        return pd.concat(dataframes)

    def groupAndAggregate(self, inputParameters: list, outputParameters: list):
        map = {param: ['mean', 'std'] for param in outputParameters}
        return self.dataframe.groupby(inputParameters).agg(map)

