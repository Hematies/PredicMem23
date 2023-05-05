
from XMLFile import XMLFile

file = XMLFile("perlbench_s_distrib_buffer_svm_.xml")
table = file.getTraceLevelDataframe()

print("")