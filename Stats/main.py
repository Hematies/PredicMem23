
from XMLFile import XMLFile
from MultiXMLReader import MultiXMLReader
from Trace import Trace

# file = XMLFile("perlbench_s_distrib_buffer_svm_.xml")
# table = file.getTraceLevelDataframe()

multiReader = MultiXMLReader("files/")

perlbench = Trace("perlbench_s", multiReader.dataframe)
cactu = Trace("cactuBSSN_s", multiReader.dataframe)
mcf = Trace("mcf_s", multiReader.dataframe)

perlbench.plotCachePerformance()
perlbench.plotDictionaryPerformance()

print("")