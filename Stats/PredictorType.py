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


class PredictorType:

    def __init__(self, predictorTypeName):
        self.predictorTypeName = predictorTypeName
        self.predicates = None

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

