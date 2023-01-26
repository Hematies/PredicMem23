// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"
#include "BuffersSimulator.h"
#include "TraceReader.h"

// string nombreFicheroDatos = "foto.jpg";//  "datasetClases.bmp"; // // "..\\..\\datos\\datasetClases.bmp";
// string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\datasetClases2.bmp";
// string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\omnetpp_s_0.bmp";
//string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\mcf_s_0.bmp";

// string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredicMem22\\trazas\\pinatrace_lbm.out";

// string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredicMem22\\trazas\\pinatrace_mcf.out";

string nombreFicheroDatos = "D:\\TrazasSPEC\\benchspec\\CPU\\605.mcf_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
//string nombreFicheroDatos = "D:\\TrazasSPEC\\benchspec\\CPU\\620.omnetpp_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
// string nombreFicheroDatos = "D:\\TrazasSPEC\\benchspec\\CPU\\600.perlbench_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
// string nombreFicheroDatos = "D:\\TrazasSPEC\\benchspec\\CPU\\625.x264_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";


/*
extern BuffersSimulator<long, long, int, long>
proposedBuffersSimulator(AccessesDataset<long, long>& dataset, BuffersDataset<int>& classesDataset,
    int numHistoryAccesses, int numClasses,
    int maxConfidence, int numConfidenceJumps);
    */

int main()
{
    
    AccessesDataset<L64b, L64b> dataset
    {
        vector<L64b>{0,2,0,2,0,2,0,2,4,8,10,12,14},
        vector<L64b>{0,1,0,1,0,1,0,1,2,2,2,3,3}
    };

    TraceReader<L64b, L64b> reader(nombreFicheroDatos);
    dataset = reader.readNextLines(10000000);// 10000000);
    // dataset = reader.readAllLines();

    BuffersDataset<int> res;
    auto b = proposedBuffersSimulator(dataset, res, 8, 6);
    
    PredictorSVM<MultiSVMClassifierOneToAll, int> predictor = 
        PredictorSVM<MultiSVMClassifierOneToAll, int>(res, 8, 6);
    predictor.simular();
}

