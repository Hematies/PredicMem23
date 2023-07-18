import math
import operator as op
from copy import copy

import pandas as pd


class Predicate:
    def __init__(self, variable: str, operator, value: object):
        self.variable = variable
        self.operator = operator
        self.value = value

    def check(self, dictionary):
        if not self.variable in dictionary:
            return False
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


defaultOrderLevels = [['Infinite', 'Real'], ['DFCMHashOnHash', 'GradeK', 'SVM']]
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
        

InfiniteBufferSVM = PredictorType("Infinite BufferSVM")
InfiniteBufferSVM.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.lt, 0),
            "or",
            ListOfPredicates([
                Predicate("numIndexBits", op.le, 0),
                Predicate("numWays", op.le, 0),
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
            Predicate("numSequenceAccesses", op.le, 0)
        ]
    )
)

InfiniteDFCMGradeK = PredictorType("Infinite DFCM GradeK")
InfiniteDFCMGradeK.setPredicates(
    ListOfPredicates(
        [
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.gt, 0)
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

        ]
    )
)
RealDFCM.setAttributes(['firstTableNumIndexBits', 'firstTableNumWays', 'secondTableNumIndexBits', 'secondTableNumWays'])

RealDFCMGradeK = PredictorType("Real DFCM GradeK")
RealDFCMGradeK.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("numIndexBits", op.gt, 0),
                "or",
                Predicate("numWays", op.gt, 0),
            ]),
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.gt, 0),
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