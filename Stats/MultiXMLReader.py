import pandas as pd

from XMLFile import XMLFile
import os

class MultiXMLReader:

    def __init__(self, directoryPath):
        self.directoryPath = directoryPath
        self.files = self.getXMLFiles(directoryPath)
        self.dataframe = self.buildDataframe(self.files)
        self.dataframe['modelHitRate'] = \
            self.dataframe['hitRate'] / (
                        (1 - self.dataframe['cacheMissRate']) * (1 - self.dataframe['dictionaryMissRate']))

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
        for file in files:
            dataframes.append(file.getTraceLevelDataframe())
        return pd.concat(dataframes)

    def groupAndAggregate(self, inputParameters: list, outputParameters: list):
        map = {param: ['mean', 'std'] for param in outputParameters}
        return self.dataframe.groupby(inputParameters).agg(map)

