
from XMLFile import XMLFile
from MultiXMLReader import MultiXMLReader
from Trace import ExploratoryTrace, WholeTrace, TraceComparer
from matplotlib import pyplot as plt

######
multiReader = MultiXMLReader("PruebasChampSim/")
comparator = TraceComparer(multiReader.dataframe)
comparator.plotPerformanceComparison("hitRate", onlyAggMean=False)
comparator.plotPerformanceComparison("totalMemoryCost", onlyAggMean=False)

######


# file = XMLFile("perlbench_s_distrib_buffer_svm_.xml")
# table = file.getTraceLevelDataframe()

multiReader = MultiXMLReader("pruebasParciales/")
perlbench = ExploratoryTrace("perlbench_s", multiReader.dataframe)
cactu = ExploratoryTrace("cactuBSSN_s", multiReader.dataframe)
mcf = ExploratoryTrace("mcf_s", multiReader.dataframe)

# mcf.plotCachePerformance()
# mcf.plotDictionaryPerformance()
# mcf.plotPredictorModelPerformance()

cactuModelHitRate_ = cactu.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['modelHitRate'])
mcfModelHitRate_ = mcf.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['modelHitRate'])
perlbenchModelHitRate_ = perlbench.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['modelHitRate'])

cactuHitRate_ = cactu.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['hitRate'])
mcfHitRate_ = mcf.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['hitRate'])
perlbenchHitRate_ = perlbench.groupAndAggregate(['numSequenceAccesses', 'numClasses'], ['hitRate'])

cactuDictionary_ = cactu.groupAndAggregate(['numClasses'], ['dictionaryMissRate', 'dictionaryMemoryCost'])
mcfDictionary_ = mcf.groupAndAggregate(['numClasses'], ['dictionaryMissRate', 'dictionaryMemoryCost'])
perlbenchDictionary_ = perlbench.groupAndAggregate(['numClasses'], ['dictionaryMissRate', 'dictionaryMemoryCost'])


cactuCacheMissRate_ = cactu.groupAndAggregate(['numIndexBits', 'numWays',], ['cacheMissRate'])
mcfCacheMissRate_ = mcf.groupAndAggregate(['numIndexBits', 'numWays'], ['cacheMissRate'])
perlbenchCacheMissRate_ = perlbench.groupAndAggregate(['numIndexBits', 'numWays'], ['cacheMissRate'])

cactuCacheMemoryCost_ = cactu.groupAndAggregate(['numIndexBits', 'numWays','numSequenceAccesses','numClasses'], ['cacheMemoryCost'])
mcfCacheMemoryCost_ = mcf.groupAndAggregate(['numIndexBits', 'numWays','numSequenceAccesses','numClasses'], ['cacheMemoryCost'])
perlbenchCacheMemoryCost_ = perlbench.groupAndAggregate(['numIndexBits', 'numWays','numSequenceAccesses','numClasses'], ['cacheMemoryCost'])

configExplorationFilesDir =  "configExplorationFiles/"
testOnAllAppsDir = "testOnAllApps/"


multiReader = MultiXMLReader("testOnAllApps/CandidatosAnalizar_")
realSVM = WholeTrace("Real DFCM K-order", multiReader.dataframe)
realSVM.plotPerformanceByTraceName(False, "Real DFCM K-order precision")

realSVM = WholeTrace("Real DFCM HoH", multiReader.dataframe)
realSVM.plotPerformanceByTraceName(False, "Real DFCM HoH precision")

realSVM = WholeTrace("Real SVM4AP", multiReader.dataframe)
realSVM.plotPerformanceByTraceName(False, "Real SVM4AP precision")



multiReader_ = MultiXMLReader(testOnAllAppsDir, onlyPredictorsOfTypes=
["Real SVM4AP", "Real DFCM HoH",# ])
"Infinite SVM4AP", "Infinite DFCM HoH"])
comparator = TraceComparer(multiReader_.dataframe)
comparator.plotParettoFront("totalMemoryCost", "hitRate", False, True, True, False, plotIdealMetric2=False)
comparator.plotPerformanceComparison("hitRate")
comparator.plotPerformanceComparison("totalMemoryCost")
comparator.plotPerformanceComparison("yield")
hitRateResults = multiReader_.groupAndAggregate(['predictorPrettyName'], ['hitRate'])
memoryResults = multiReader_.groupAndAggregate(['predictorPrettyName'], ['totalMemoryCost'])


# multiReader = MultiXMLReader("testOnAllApps/DFCMOrdenK")
multiReader = MultiXMLReader(testOnAllAppsDir, onlyPredictorsOfTypes=
["Real DFCM HoH"])
comparator = TraceComparer(multiReader.dataframe)
comparator.plotPerformanceComparison("hitRate", onlyAggMean=False)
comparator.plotPerformanceComparison("totalMemoryCost", onlyAggMean=True)
hitRateResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['hitRate'])
memoryResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['totalMemoryCost'])

multiReader = MultiXMLReader("testOnAllApps/DFCMOrdenK",)
comparator = TraceComparer(multiReader.dataframe)
hitRateResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['hitRate'])
memoryResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['totalMemoryCost'])
comparator.plotPerformanceComparison("hitRate", onlyAggMean=False)
comparator.plotPerformanceComparison("totalMemoryCost", onlyAggMean=True)

multiReader = MultiXMLReader(testOnAllAppsDir, onlyPredictorsOfTypes=
["Real SVM4AP"])
comparator = TraceComparer(multiReader.dataframe)
comparator.plotPerformanceComparison("hitRate", onlyAggMean=True)
comparator.plotPerformanceComparison("totalMemoryCost", onlyAggMean=True)


multiReader = MultiXMLReader(testOnAllAppsDir)

comparator = TraceComparer(MultiXMLReader(testOnAllAppsDir).dataframe)
comparator.plotParettoFront("totalMemoryCost", "hitRate", False, True, True)
comparator.plotPerformanceComparison("hitRate")
comparator.plotPerformanceComparison("totalMemoryCost")
comparator.plotPerformanceComparison("yield")
hitRateResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['hitRate'])
memoryResults = multiReader.groupAndAggregate(['predictorPrettyName'], ['totalMemoryCost'])

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

'''
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
'''



print("")