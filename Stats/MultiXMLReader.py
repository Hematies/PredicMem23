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

