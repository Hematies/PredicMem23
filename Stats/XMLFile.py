import xml.etree.ElementTree as ET
import os
import xmltodict
import pandas as pd

globalColumns = [
    "experimentationFilename",
    "predictorType",
    "traceName",
    # "dateTime",
    "firstAccess", # INCLUSIVE
    "lastAccess", # NOT INCLUSIVE
    "hitRate",
    "totalMemoryCost"
]

cacheColumns = [
    "numIndexBits",
    "numWays",
    "numSequenceAccesses",
    "saveHistoryAndClassIfNotValid",
    "cacheMemoryCost",
    "cacheMissRate"
]

dictColumns = [
    "numClasses",
    "maxConfidence",
    "numConfidenceJumps",
    "saveHistoryAndClassIfNotValid",
    "dictionaryMemoryCost",
    "dictionaryMissRate"
]

DFCMColumns = [
    "firstTableMissRate",
    "secondTableMissRate",
    "firstTableMemoryCost",
    "secondTableMemoryCost"
]

modelColumns = [
    "modelMemoryCost",
]

allColumns = []
allColumns.extend(globalColumns)
allColumns.extend(cacheColumns)
allColumns.extend(dictColumns)
allColumns.extend(DFCMColumns)
allColumns.extend(modelColumns)

stringColumns = [
    "experimentationFilename",
    "predictorType",
    "traceName"
]
nanString = "-nan(ind)"

def initDictTable():
    res = dict()
    for column in allColumns:
        res[column] = []

    return res

def initTableRow():
    res = dict()
    for column in allColumns:
        res[column] = ""

    return res

def addRowsToTableDict(tableDict: dict, rows: list):
    res = dict(tableDict)
    for column in res.keys():
        C = res[column]
        for row in rows:
            value = ""
            if column in row:
                value = row[column]
            C.append(value)
        res[column] = C
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
        return pd.DataFrame(data=dictTable)