import math
import xml.etree.ElementTree as ET
import os
import xmltodict
import pandas as pd
from PredictorType import InfiniteBufferSVM, RealBufferSVM, InfiniteDFCM, InfiniteDFCMGradeK, RealDFCM, RealDFCMGradeK, \
    PredictorsHelper, defaultOrderLevels, possiblePredictorTypes, allColumns


def initDictTable():
    res = dict()
    for column in allColumns:
        res[column[0]] = []

    return res

def initTableRow():
    res = dict()
    for column in allColumns:
        t = column[1]
        if t == "string":
            res[column[0]] = ""
        elif t == "int": # Natural number is assumed
            res[column[0]] = -1
        elif t == "float":
            res[column[0]] = math.nan
        elif t == "bool":
            res[column[0]] = False
        else:
            raise Exception("One type more is missing")

    return res

def addRowsToTableDict(tableDict: dict, rows: list):
    res = dict(tableDict)
    for column in res.keys():
        C = res[column]
        for row in rows:
            value = ""
            if column in row:
                value = row[column]
                value = fixOnWrongValue(value)
            C.append(value)
        res[column] = C
    return res

def setTypeToDataframe(dataframe: pd.DataFrame):
    columnsAndTypes = dict()
    for column in allColumns:
        columnsAndTypes[column[0]] = column[1]
    return dataframe.astype(columnsAndTypes)

def setTypeToTableDict(tableDict: dict):
    dataframe = setTypeToDataframe(pd.DataFrame(data=tableDict))
    return dataframe.to_dict("list")


def fixOnWrongValue(value):
    res = value
    # 1) Check if value == "-nan(ind)"
    if(value == "-nan(ind)"):
        res = "NaN"
    return res

def checkAndSetPredictorType(dictTable):
    res = dict(dictTable)
    for predictorType in possiblePredictorTypes:
        res = predictorType.setPredictorType(res)
    return res

class XMLFile:

    def __init__(self, path):
        self.directory = os.path.dirname(path)
        self.filename = os.path.splitext(path)[0]
        self.path = path
        # self.tree = ET.parse(path)
        # self.root = self.tree.getroot()
        with open(path) as fd:
            self.dictionary = xmltodict.parse(fd.read())

    '''
    # Pfff... esto hay que dejarlo para luego:
    def getWholeDataframe(self):
        dictTable = initDictTable()
        for traces in self.dictionary.values():
            for trace in traces.items():
                row = dict()
                traceName = trace[0]
                row['traceName'] = traceName
                content = trace[1]
                for info in content.items():
                    # It is an attribute:
                    if info[0][0] == '@':
                        attribute = info[0][1:]
                        value = info[1]
                        row[attribute] = value
                    # It is a trace part:
                    else:
                        part = info[1]
                        for block in
    '''
    def getTraceLevelDataframe(self):
        dictTable = initDictTable()
        rows = []
        for traces in self.dictionary.values():
            for trace in traces.items():
                row = initTableRow()
                traceName = trace[0]
                row['traceName'] = traceName
                content = trace[1]
                for info in content.items():
                    # It is an attribute:
                    if info[0][0] == '@':
                        attribute = info[0][1:]
                        value = info[1]
                        row[attribute] = value
                    # It is a trace part:
                    else:
                        part = info[1]
                        for block in part.items():
                            # We take only the predictor parameters once...
                            # ... because they were embedded inside experiments:
                            if block[0] != "resultsAndCosts":
                                for param in block[1].items():
                                    attribute = param[0][1:]
                                    value = param[1]
                                    row[attribute] = value

                rows.append(row)
        addRowsToTableDict(dictTable, rows)
        dictTable = setTypeToTableDict(dictTable)
        dictTable = checkAndSetPredictorType(dictTable)
        return pd.DataFrame(data=dictTable)