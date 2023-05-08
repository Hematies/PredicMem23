import pandas as pd

from XMLFile import XMLFile
import os

class MultiXMLReader:

    def __init__(self, directoryPath):
        self.directoryPath = directoryPath
        self.files = self.getXMLFiles(directoryPath)
        self.dataframe = self.buildDataframe(self.files)

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

