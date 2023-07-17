import operator as op
from copy import copy
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


defaultOrderLevels = [['Infinite', 'Real'], ['DFCM HashOnHash', 'order', 'SVM']]
class PredictorsHelper:
    def __init__(self, predictorTypes: list, orderLevels: list = defaultOrderLevels):
        self.predictorTypes = {predictor.predictorTypeName: predictor for predictor in predictorTypes}
        self.orderLevels = orderLevels

    def getOrderedPredictors(self, groupsSemiOrderedPredictors: dict, orderLevels):
        res_ = groupsSemiOrderedPredictors

        # First case: there are more than one order level left (of more priority), so we send the work to the stack and
        # receive and intermediary result.
        if len(orderLevels) > 1:
            res_ = self.getOrderedPredictors(groupsSemiOrderedPredictors, list(orderLevels[0:-1]))
        # The results are groups of ordered predictors (dictionary) which have to be reordered separately:
        res__ = []
        orderLevel = orderLevels[-1]
        orders = {k: orderLevel[k] for k in range(0, len(orderLevel))}
        for group in res_:
            newGroups = []
            for order in orders.items():
                newGroup = [predictor for predictor in group if order[1] in predictor.predictorTypeName]
                newGroup = sorted(newGroup, key = lambda p: p.predictorTypeName)
                newGroups.append(newGroup)
            res__.extend(newGroups)

        # Second case: this was the last sort, so the final result is the merge of all sorted groups:
        if len(orderLevels) <= 1:
            return [element for element in group for group in res__]
        else:
            return res__



    def getPredictorFamily(self, predictorType):
        name = predictorType.predictorTypeName
        res = name
        if 'Real' in name:
            res = name[name.find('Real') + len('Real') :]
        elif 'Infinite' in name:
            res = name[name.find('Infinite') + len('Infinite'):]
        if '_' in res:
            res = res.split('_')[0].strip()
        return res

    def getPredictorTranslation(self, predictorType):
        realAttribute = 'Real' if self.isPredictorReal(predictorType) else 'Infinite'
        family = self.getPredictorFamily(predictorType)
        attributes = self.getPredictorAttributes(predictorType)
        res = realAttribute + ' ' + family + ' '
        for k in range(0, len(attributes.values())):
            res = res + attributes.values()[k]
            if k < len(attributes.values()) - 1:
                res = res + '-'
        return res

    def getPredictorAttributes(self, predictorType):
        res = list(predictorType.attributes)
        predictorTypeName:str = predictorType.predictorTypeName
        parts = predictorTypeName.split('_')[1:]
        return {predictorType.attributes[k]: int(parts[k].strip()) for k in range(0, len(parts))}


    def isPredictorReal(self, predictorTypeName):
        return ("Real" in predictorTypeName)

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
                        surname = surname + '_' + tableDict[column][i]
        return res

InfiniteBufferSVM = PredictorType("InfiniteBufferSVM")
InfiniteBufferSVM.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.lt, 0),
            "or",
            ListOfPredicates([
                Predicate("numIndexBits", op.le, 0),
                Predicate("numWays", op.le, 0),
            ])
        ]
    )
)

RealBufferSVM = PredictorType("RealBufferSVM")
RealBufferSVM.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("numIndexBits", op.gt, 0),
                "or",
                Predicate("numWays", op.gt, 0),
            ]),
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
        ]
    )
)

RealBufferSVM_4_4 = PredictorType("RealBufferSVM_4_4")
RealBufferSVM_4_4.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("numIndexBits", op.gt, 0),
                "or",
                Predicate("numWays", op.gt, 0),
            ]),

            # In order to differentiate with RealBufferSVM of 4-length sequences and 4 classes:
            ListOfPredicates([
                Predicate("numSequenceAccesses", op.eq, 4),
                "and",
                Predicate("numClasses", op.eq, 4),
            ]),
        ]
    )
)

RealBufferSVM_8_8 = PredictorType("RealBufferSVM_8_8")
RealBufferSVM_8_8.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("numIndexBits", op.gt, 0),
                "or",
                Predicate("numWays", op.gt, 0),
            ]),
            # In order to differentiate with RealBufferSVM of 8-length sequences and 8 classes:
            ListOfPredicates([
                Predicate("numSequenceAccesses", op.eq, 8),
                "and",
                Predicate("numClasses", op.eq, 8),
            ]),
        ]
    )
)

# DFCMs of infinite size:
InfiniteDFCM = PredictorType("InfiniteDFCM")
InfiniteDFCM.setPredicates(
    ListOfPredicates(
        [
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.le, 0)
        ]
    )
)

InfiniteDFCMGradeK = PredictorType("InfiniteDFCMGradeK")
InfiniteDFCMGradeK.setPredicates(
    ListOfPredicates(
        [
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.gt, 0)
        ]
    )
)

# Real DFCMs:
RealDFCM = PredictorType("RealDFCM")
RealDFCM.setPredicates(
    ListOfPredicates(
        [
            Predicate("numIndexBits", op.ge, 0),
            ListOfPredicates([
                Predicate("numIndexBits", op.gt, 0),
                "or",
                Predicate("numWays", op.gt, 0),
            ]),
            Predicate("firstTableMemoryCost", op.ge, 0),
            Predicate("numSequenceAccesses", op.le, 0),

        ]
    )
)

RealDFCMGradeK = PredictorType("RealDFCMGradeK")
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

predictorHelper = PredictorsHelper([
    InfiniteBufferSVM,
    RealBufferSVM,
    RealBufferSVM_4_4,
    RealBufferSVM_8_8,
    InfiniteDFCM,
    InfiniteDFCMGradeK,
    RealDFCM,
    RealDFCMGradeK,
], defaultOrderLevels)

