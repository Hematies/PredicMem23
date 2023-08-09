
from XMLFile import XMLFile
from MultiXMLReader import MultiXMLReader
from Trace import ExploratoryTrace, WholeTrace, TraceComparer
from matplotlib import pyplot as plt

# file = XMLFile("perlbench_s_distrib_buffer_svm_.xml")
# table = file.getTraceLevelDataframe()

configExplorationFilesDir =  "configExplorationFiles/"
testOnAllAppsDir = "testOnAllApps/"

multiReader_ = MultiXMLReader(testOnAllAppsDir, onlyPredictorsOfTypes=["Real DFCM K-order"])
comparator = TraceComparer(multiReader_.dataframe)
comparator.plotParettoFront("totalMemoryCost", "hitRate", False, True, True)
comparator.plotPerformanceComparison("hitRate")
comparator.plotPerformanceComparison("totalMemoryCost")
comparator.plotPerformanceComparison("yield")

multiReader = MultiXMLReader(testOnAllAppsDir)

comparator = TraceComparer(MultiXMLReader(testOnAllAppsDir).dataframe)
comparator.plotParettoFront("totalMemoryCost", "hitRate", False, True, True)
comparator.plotPerformanceComparison("hitRate")
comparator.plotPerformanceComparison("totalMemoryCost")
comparator.plotPerformanceComparison("yield")

perlbench = ExploratoryTrace("perlbench_s", multiReader.dataframe)
cactu = ExploratoryTrace("cactuBSSN_s", multiReader.dataframe)
mcf = ExploratoryTrace("mcf_s", multiReader.dataframe)

# mcf.plotCachePerformance()
# mcf.plotDictionaryPerformance()
# mcf.plotPredictorModelPerformance()

cactu_ = cactu.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['modelHitRate'])
mcf_ = mcf.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['modelHitRate'])
perlbench_ = perlbench.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['modelHitRate'])

# cactu_ = cactu.groupAndAggregate(['numClasses'], ['dictionaryMissRate', 'dictionaryMemoryCost'])
# mcf_ = mcf.groupAndAggregate(['numClasses'], ['dictionaryMissRate', 'dictionaryMemoryCost'])
# perlbench_ = perlbench.groupAndAggregate(['numClasses'], ['dictionaryMissRate', 'dictionaryMemoryCost'])


multiReader = MultiXMLReader(testOnAllAppsDir)

yieldResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['yield'])
hitRateResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['hitRate'])
memoryResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['totalMemoryCost'])
# firstTableHitRateResults = multiReader.groupAndAggregate(['predictorType'], ['firstTableHitRate'])
# secondTableHitRateResults = multiReader.groupAndAggregate(['predictorType'], ['secondTableHitRate'])
dictionaryHitRateResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['dictionaryMissRate'])
svmHitRateResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['modelHitRate'])

idealSVM = WholeTrace("InfiniteBufferSVM", multiReader.dataframe)
idealSVM.plotPerformanceByTraceName(False, "Ideal BufferSVM precision")
idealSVM.plotPerformanceByTraceName(True, "Ideal BufferSVM memory cost")
# resIdealSVM = idealSVM.groupAndAggregate([], ['hitRate'])

idealDFCM = WholeTrace("InfiniteDFCM", multiReader.dataframe)
idealDFCM.plotPerformanceByTraceName(False, "Ideal HashOnHash DFCM precision")
idealDFCM.plotPerformanceByTraceName(True, "Ideal HashOnHash DFCM memory cost")
# resIdealDFCM = idealDFCM.groupAndAggregate([], ['hitRate'])

idealDFCMGradeK = WholeTrace("InfiniteDFCMGradeK", multiReader.dataframe)
idealDFCMGradeK.plotPerformanceByTraceName(False, "Ideal 8-order DFCM precision")
idealDFCMGradeK.plotPerformanceByTraceName(True, "Ideal 8-order DFCM memory cost")
# resIdealDFCMGradeK = idealDFCMGradeK.groupAndAggregate([], ['hitRate'])



multiReader = MultiXMLReader(testOnAllAppsDir)
realSVM = WholeTrace("RealBufferSVM_8_8", multiReader.dataframe)
realSVM.plotPerformanceByTraceName(False, "Real BufferSVM precision")

realSVM = WholeTrace("RealBufferSVM_4_4", multiReader.dataframe)
realSVM.plotPerformanceByTraceName(False, "Real BufferSVM precision")

comparator = TraceComparer(MultiXMLReader(testOnAllAppsDir).dataframe)
comparator.plotPerformanceComparison("hitRate")
# comparator.plotPerformanceComparison("totalMemoryCost")

print("")