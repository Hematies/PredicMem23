import math
import operator as op
from copy import copy

import pandas as pd

globalColumns = [
    ("experimentationFilename", "string"),
    ("predictorType","string"),
    ("predictorPrettyName","string"),
    ("traceName","string"),
    # "dateTime",
    ("firstAccess","int"), # INCLUSIVE
    ("lastAccess","int"), # NOT INCLUSIVE
    ("hitRate","float"),
    ("totalMemoryCost", "float")
]

cacheColumns = [
    ("numIndexBits","int"),
    ("numWays","int"),
    ("numSequenceAccesses","int"),
    ("saveHistoryAndClassIfNotValid","bool"),
    ("cacheMemoryCost","float"),
    ("cacheMissRate","float"),
]

dictColumns = [
    ("numClasses","int"),
    ("maxConfidence","int"),
    ("numConfidenceJumps","int"),
    ("saveHistoryAndClassIfNotValid","bool"),
    ("dictionaryMemoryCost","float"),
    ("dictionaryMissRate","float"),
]

DFCMColumns = [
    ("firstTableNumIndexBits","int"),
    ("firstTableNumWays","int"),
    ("secondTableNumIndexBits","int"),
    ("secondTableNumWays","int"),
    ("firstTableMissRate","float"),
    ("secondTableMissRate","float"),
    ("firstTableMemoryCost","float"),
    ("secondTableMemoryCost","float"),
]

modelColumns = [
    ("modelMemoryCost","float"),
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




class Predicate:
    def __init__(self, variable: str, operator, value: object):
        self.variable = variable
        self.operator = operator
        self.value = value

    def check(self, dictionary):
        if not self.variable in dictionary:
            return False
        else:
            if self.operator == op.eq and math.isnan(self.value):
                return math.isnan(dictionary[self.variable])
            elif self.operator == op.ne and math.isnan(self.value):
                return not math.isnan(dictionary[self.variable])
            else:
                return self.operator(dictionary[self.variable], self.value)

class ListOfPredicates:
    def __init__(self, listOfPredictates: list):
        self.predicates = copy(listOfPredictates)

    def check(self, dictionary):
        res = True
        currentLogicalOp = "and"
        for predicateOrOp in self.predicates:
            if isinstance(predicateOrOp, Predicate) or isinstance(predicateOrOp, ListOfPredicates):
                if currentLogicalOp == "and":
                    res = res and predicateOrOp.check(dictionary)
                elif currentLogicalOp == "or":
                    res = res or predicateOrOp.check(dictionary)
                else:
                    raise Exception("Unknown logical operation.")
                # currentLogicalOp = "and" # Default operation
            elif isinstance(predicateOrOp,str):
                currentLogicalOp = predicateOrOp
            else:
                raise Exception("Unknown logical operation or predicate.")
        return res


defaultOrderLevels = [['Infinite', 'Real'], ['HashOnHash', 'GradeK', 'SVM']]
class PredictorsHelper:
    def __init__(self, predictorTypes: list, orderLevels: list = defaultOrderLevels):
        self.predictorTypes = {predictor.predictorTypeName: predictor for predictor in predictorTypes}
        self.orderLevels = orderLevels
        self.predictors = {}

    def setPredictors(self, tableDict: dict):
        numRows = len(list(tableDict.values())[0])
        res = []
        for i in range(0, numRows):
            rowDictionary = dict()
            for column in tableDict.items():
                rowDictionary[column[0]] = column[1][i]
            for predictorType in self.predictorTypes.values():
                if predictorType.predicates.check(rowDictionary):
                    res.append(Predictor(predictorType, rowDictionary))
                    # break
        self.predictors = {predictor.getPredictorTranslation(): predictor for predictor in res}

    def getOrderedPredictors(self, groupsSemiOrderedPredictors: [], orderLevels, returnResult = True):
        res_ = groupsSemiOrderedPredictors

        # First case: there are more than one order level left (of more priority), so we send the work to the stack and
        # receive and intermediary result.
        if len(orderLevels) > 1:
            res_ = self.getOrderedPredictors(res_, list(orderLevels[0:-1]), False)
        # The results are groups of ordered predictors (dictionary) which have to be reordered separately:
        res__ = []
        orderLevel = orderLevels[-1]
        orders = {k: orderLevel[k] for k in range(0, len(orderLevel))}
        for group in res_:
            newGroups = []
            for order in orders.items():
                if isinstance(group, dict):
                    g = list(group.values())
                else:
                    g = list(group)
                newGroup = [predictor for predictor in g if order[1] in predictor.getPredictorTranslation()]
                newGroup = sorted(newGroup, key = lambda p: p.getPredictorTranslation())
                newGroups.append(newGroup)
            res__.extend(newGroups)

        # Second case: this was the last sort, so the final result is the merge of all sorted groups:
        if returnResult:
            res = []
            for group_ in res__:
                for element in group_:
                    res.append(element)
            return res
        else:
            return res__

    def getPredictorsOrder(self, predictors: list):
        orderedPredictors = self.getOrderedPredictors([predictors], self.orderLevels)
        return {orderedPredictors[i].getPredictorTranslation(): i for i in range(0,len(orderedPredictors))}

    def setPredictorsNameAndTranslatedToDataframe(self, dataframe: pd.DataFrame):
        tableDict = dataframe.to_dict("list")
        numRows = len(list(tableDict.values())[0])
        res = []
        for i in range(0, numRows):
            rowDictionary = dict()
            for column in tableDict.items():
                rowDictionary[column[0]] = column[1][i]
            for predictorType in self.predictorTypes.values():
                if predictorType.predicates.check(rowDictionary):
                    predictor = Predictor(predictorType, rowDictionary)
                    # break
            rowDictionary["predictorType"] = predictor.predictorType.predictorTypeName
            rowDictionary["predictorPrettyName"] = predictor.getPredictorTranslation()
            res.append(rowDictionary)
        return pd.DataFrame(res)

    def setPredictorsMemoryCostsToDataframe(self, dataframe: pd.DataFrame):
        tableDict = dataframe.to_dict("list")
        numRows = len(list(tableDict.values())[0])
        res = []
        for i in range(0, numRows):
            rowDictionary = dict()
            for column in tableDict.items():
                rowDictionary[column[0]] = column[1][i]
            for predictorType in self.predictorTypes.values():
                if predictorType.predicates.check(rowDictionary):
                    predictor = Predictor(predictorType, rowDictionary)
                    # break

            for memoryCostType, memoryCost in predictor.getPredictorMemoryCosts(rowDictionary).items():
                rowDictionary[memoryCostType] = memoryCost
            res.append(rowDictionary)
        return pd.DataFrame(res)

    # def getPredictorsTranslationTable(self):

    '''
    def getPredictorAttributes(self, predictorType):
        res = list(predictorType.attributes)
        predictorTypeName:str = predictorType.predictorTypeName
        parts = predictorTypeName.split('_')[1:]
        return {predictorType.attributes[k]: int(parts[k].strip()) for k in range(0, len(parts))}
    '''

class PredictorType:

    def __init__(self, predictorTypeName):
        self.predictorTypeName = predictorTypeName
        self.predicates = None
        self.attributes = []

    def setAttributes(self, attributes: list):
        self.attributes = list(attributes)

    def setPredicates(self, predicates: ListOfPredicates):
        self.predicates = copy(predicates)

    def checkPredictorType(self, tableDict: dict):
        res = []
        numRows = len(list(tableDict.values())[0])
        for i in range(0, numRows):
            rowDictionary = dict()
            for column in tableDict.items():
                rowDictionary[column[0]] = column[1][i]
            isPredictor = self.predicates.check(rowDictionary)
            res.append(isPredictor)
        return res

    def setPredictorType(self, tableDict: dict):
        res = dict(tableDict)
        mask = self.checkPredictorType(tableDict)
        numRows = len(mask)
        for i in range(0, numRows):
            if mask[i]:
                res["predictorType"][i] = self.predictorTypeName
                surname = ""
                for column in tableDict.keys():
                    if str(column) in self.attributes:
                        surname = surname + '_' + str(tableDict[column][i])
        return res

    def isPredictorReal(self):
        return ("Real" in self.predictorTypeName)

    def getPredictorFamily(self):
        name = self.predictorTypeName
        res = name
        if 'Real' in name:
            res = name[name.find('Real') + len('Real') :]
        elif 'Infinite' in name:
            res = name[name.find('Infinite') + len('Infinite'):]
        if '_' in res:
            res = res.split('_')[0].strip()
        return res

def computeCacheNumEntries(numIndexBits: int, numWays: int):
    return math.pow(2, numIndexBits) * numWays

def computeEntryCapacity(numWords: int, numWordBits=64, numSequenceElements = -1, numSVMClasses = -1):
    numWordsBits = numWords * numWordBits
    numSequenceBits = 0
    if numSequenceElements >= 0 and numSVMClasses >= 0:
        numTotalClasses = numSVMClasses + 1
        numClassBits = math.ceil(math.log2(numTotalClasses))
        numSequenceBits = numClassBits * numSequenceElements
    elif numSequenceElements >= 0:
        numSequenceBits = numWordBits * numSequenceElements
    numTotalBits = numWordsBits + numSequenceBits
    return float(numTotalBits) / 8.0

def computeEntryCost(numIndexBits: int, numWords: int, numWordBits=64, numSequenceElements = -1, numSVMClasses = -1):
    numTagBits = numWordBits - numIndexBits
    return (float(numTagBits) / 8.0) + computeEntryCapacity(numWords, numWordBits, numSequenceElements, numSVMClasses)

def computeCacheCapacity(numIndexBits: int, numWays: int, numWords: int, numWordBits=64, numSequenceElements = -1, numSVMClasses = -1):
    return computeCacheNumEntries(numIndexBits,numWays) * \
           computeEntryCapacity(numIndexBits, numWords, numWordBits, numSequenceElements, numSVMClasses)

def computeCacheCost(numIndexBits: int, numWays: int, numWords: int, numWordBits=64, numSequenceElements = -1, numSVMClasses = -1):
    return computeCacheNumEntries(numIndexBits,numWays) * \
           computeEntryCost(numIndexBits, numWords, numWordBits, numSequenceElements, numSVMClasses)

class Predictor:
    def __init__(self, predictorType: PredictorType, data: dict):
        self.predictorType = predictorType
        self.attributes = {key: data[key] for key in self.predictorType.attributes}

    def getPredictorTranslation(self):
        realAttribute = 'Real' if self.predictorType.isPredictorReal() else 'Infinite'
        family = self.predictorType.getPredictorFamily()
        attributes = self.attributes
        res = realAttribute + ' ' + family + ' '
        for k in range(0, len(attributes.values())):
            res = res + str(list(attributes.values())[k])
            if k < len(attributes.values()) - 1:
                res = res + '-'
        return res

    def setPredictorTranslatedName(self, tableDict: dict):
        res = dict(tableDict)
        mask = self.checkPredictorType(tableDict)
        numRows = len(mask)
        for i in range(0, numRows):
            if mask[i]:
                res["predictorType"][i] = self.predictorTypeName
                surname = ""
                for column in tableDict.keys():
                    if str(column) in self.attributes:
                        surname = surname + '_' + str(tableDict[column][i])
        return res

    def getPredictorMemoryCosts(self, rowDict: dict):
        translatedName = self.getPredictorTranslation()
        allColumns_ = list(filter(lambda t: "emoryCost" in t[0], allColumns))
        res = {column: rowDict[column] for column, type in allColumns_}
        if not 'Infinite' in translatedName:
            if 'BufferSVM' in translatedName:
                res["totalMemoryCost"] -= res["cacheMemoryCost"]
                res["cacheMemoryCost"] = \
                    computeCacheCost(
                        rowDict["numIndexBits"],
                        rowDict["numWays"],
                        1,
                        numSequenceElements=rowDict["numSequenceAccesses"],
                        numSVMClasses=rowDict["numClasses"])
                res["totalMemoryCost"] += res["cacheMemoryCost"]
            elif 'HashOnHash' in translatedName:
                res["totalMemoryCost"] -= res["firstTableMemoryCost"]
                res["firstTableMemoryCost"] = \
                    computeCacheCost(
                        rowDict["firstTableNumIndexBits"],
                        rowDict["firstTableNumWays"],
                        2)
                res["totalMemoryCost"] -= res["secondTableMemoryCost"]
                res["secondTableMemoryCost"] = \
                    computeCacheCost(
                        rowDict["secondTableNumIndexBits"],
                        rowDict["secondTableNumWays"],
                        1)
                res["totalMemoryCost"] += res["firstTableMemoryCost"] + res["secondTableMemoryCost"]
            elif 'GradeK':
                res["totalMemoryCost"] -= res["firstTableMemoryCost"]
                res["firstTableMemoryCost"] = \
                    computeCacheCost(
                        rowDict["firstTableNumIndexBits"],
                        rowDict["firstTableNumWays"],
                        0,
                        numSequenceElements=rowDict["numSequenceAccesses"],
                        )
                res["totalMemoryCost"] -= res["secondTableMemoryCost"]
                res["secondTableMemoryCost"] = \
                    computeCacheCost(
                        rowDict["secondTableNumIndexBits"],
                        rowDict["secondTableNumWays"],
                        1)
                res["totalMemoryCost"] += res["firstTableMemoryCost"] + res["secondTableMemoryCost"]
            else:
                pass
        return res

BufferSVMPredicate = Predicate("firstTableMemoryCost", op.eq, float('nan'))
DFCMPredicate = Predicate("firstTableMemoryCost", op.ne, float('nan'))
InfiniteBufferSVM = PredictorType("Infinite BufferSVM")
InfiniteBufferSVM.setPredicates(
    ListOfPredicates(
        [
            ListOfPredicates([
                Predicate("numIndexBits", op.lt, 0),
                BufferSVMPredicate]),
            "or",
            ListOfPredicates([
                Predicate("numIndexBits", op.le, 0),
                Predicate("numWays", op.le, 0),
                BufferSVMPredicate
            ]),
        ]
    )
)
InfiniteBufferSVM.setAttributes(['numSequenceAccesses', 'numClasses'])

RealBufferSVM = PredictorType("Real BufferSVM")
'''
# In order to differentiate with RealBufferSVM of 4-length sequences and 4 classes:
            ListOfPredicates([
                Predicate("numSequenceAccesses", op.ne, 4),
                "and",
                Predicate("numClasses", op.ne, 4),
            ]),
            # In order to differentiate with RealBufferSVM of 8-length sequences and 8 classes:
            ListOfPredicates([
                Predicate("numSequenceAccesses", op.ne, 8),
                "and",
                Predicate("numClasses", op.ne, 8),
            ]),
'''
RealBufferSVM.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("numIndexBits", op.gt, 0),
                "or",
                Predicate("numWays", op.gt, 0),
            ]),
            BufferSVMPredicate
        ]
    )
)
RealBufferSVM.setAttributes(['numSequenceAccesses', 'numClasses', 'numIndexBits', 'numWays'])

# DFCMs of infinite size:
InfiniteDFCM = PredictorType("Infinite DFCM HashOnHash")
InfiniteDFCM.setPredicates(
    ListOfPredicates(
        [
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.le, 0),
            DFCMPredicate
        ]
    )
)

InfiniteDFCMGradeK = PredictorType("Infinite DFCM GradeK")
InfiniteDFCMGradeK.setPredicates(
    ListOfPredicates(
        [
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.gt, 0),
            DFCMPredicate
        ]
    )
)
InfiniteDFCMGradeK.setAttributes(['numSequenceAccesses'])

# Real DFCMs:
RealDFCM = PredictorType("Real DFCM HashOnHash")
RealDFCM.setPredicates(
    ListOfPredicates(
        [
            Predicate("firstTableNumIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("firstTableNumIndexBits", op.gt, 0),
                "or",
                Predicate("firstTableNumWays", op.gt, 0),
            ]),
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.le, 0),
            DFCMPredicate
        ]
    )
)
RealDFCM.setAttributes(['firstTableNumIndexBits', 'firstTableNumWays', 'secondTableNumIndexBits', 'secondTableNumWays'])

RealDFCMGradeK = PredictorType("Real DFCM GradeK")
RealDFCMGradeK.setPredicates(
    ListOfPredicates(
        [
            Predicate("firstTableNumIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("firstTableNumIndexBits", op.gt, 0),
                "or",
                Predicate("firstTableNumWays", op.gt, 0),
            ]),
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.gt, 0),
            DFCMPredicate
        ]
    )
)
RealDFCMGradeK.setAttributes(["numSequenceAccesses", 'firstTableNumIndexBits', 'firstTableNumWays',
                              'secondTableNumIndexBits', 'secondTableNumWays'])




metricTranslationTable = {
    'hitRate': 'Predictor hit rate',
    'totalMemoryCost': "Total", # mean memory capacity",
    'cacheMemoryCost': 'Cache memory capacity',
    'cacheMissRate': 'Input buffer miss rate',
    'cacheHitRate': 'Input buffer hit rate',
    'dictionaryMemoryCost': 'Dictionary memory capacity',
    'dictionaryMissRate': 'Dictionary miss rate',
    'dictionaryHitRate': 'Dictionary hit rate',
    'firstTableMissRate': 'First table miss rate',
    'firstTableHitRate': 'First table hit rate',
    'secondTableMissRate': 'Second table miss rate',
    'secondTableHitRate': 'Second table hit rate',
    'firstTableNumIndexBits': 'First table num. index. bits',
    'firstTableNumWays': 'First table num. ways',
    'secondTableNumIndexBits': 'Second table num. index. bits',
    'secondTableWays': 'Second table num. ways',
    'firstTableMemoryCost': "First table", # 's mean memory capacity",
    'secondTableMemoryCost': "Second table", # 's mean memory capacity",
    'modelMemoryCost': 'Model memory capacity',
    'modelHitRate': 'Model hit rate',
    'predictorHitRate': 'Predictor hit rate',
    'yield': 'Hit rate / Memory cost',
}

'''
predictorTypeTranslationTable = {
    'InfiniteDFCM': 'Ideal HashOnHash DFCM',
    'InfiniteBufferSVM': 'Ideal SVM4AP',
    'RealBufferSVM': 'Real SVM4AP',
    'RealBufferSVM_4_4': 'Real SVM4AP 4-4',
    'RealBufferSVM_8_8': 'Real SVM4AP 8-8',
    'InfiniteDFCMGradeK': 'Ideal 8-order DFCM',
    'RealDFCM': 'Real HashOnHash DFCM',
    'RealDFCMGradeK': 'Real 8-order DFCM'
}

predictorTypeOrder = {
    'InfiniteDFCM': 0,
    'InfiniteBufferSVM': 2,
    'RealBufferSVM': 3,
    'RealBufferSVM_4_4': 6,
    'RealBufferSVM_8_8': 7,
    'InfiniteDFCMGradeK': 1,
    'RealDFCM': 4,
    'RealDFCMGradeK': 5
}
'''

'''
predictorTypeTranslationTable = \
    {predictor.predictorTypeName: predictorHelper.getPredictorTranslation(predictor) for predictor in predictorHelper.predictorTypes.values() }

predictorTypeOrder = predictorHelper.getOrderedPredictors([list(predictorHelper.predictorTypes.values())], defaultOrderLevels)
'''


possiblePredictorTypes =  [
    InfiniteBufferSVM,
    RealBufferSVM,
    InfiniteDFCM,
    InfiniteDFCMGradeK,
    RealDFCM,
    RealDFCMGradeK,
]

print('')