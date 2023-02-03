// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"
#include "BuffersSimulator.h"
#include "TraceReader.h"
#include "Experimentation.h"
#include "Global.h"

// string nombreFicheroDatos = "foto.jpg";//  "datasetClases.bmp"; // // "..\\..\\datos\\datasetClases.bmp";
// string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\datasetClases2.bmp";
// string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\omnetpp_s_0.bmp";
//string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\mcf_s_0.bmp";

// string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredicMem22\\trazas\\pinatrace_lbm.out";

// string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredicMem22\\trazas\\pinatrace_mcf.out";

string mcf_s = "D:\\TrazasSPEC\\benchspec\\CPU\\605.mcf_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string omnetpp_s = "D:\\TrazasSPEC\\benchspec\\CPU\\620.omnetpp_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string perlbench_s = "D:\\TrazasSPEC\\benchspec\\CPU\\600.perlbench_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string x264_s = "D:\\TrazasSPEC\\benchspec\\CPU\\625.x264_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";

string prueba2 = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredicMem22\\trazas\\trazaCCL.out";
string prueba1 = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredicMem22\\trazas\\pinatrace.out";


/*
extern BuffersSimulator<long, long, int, long>
proposedBuffersSimulator(AccessesDataset<long, long>& dataset, BuffersDataset<int>& classesDataset,
    int numHistoryAccesses, int numClasses,
    int maxConfidence, int numConfidenceJumps);
    */

int main()
{
    bool probarExperimentacion = true;
    if (!probarExperimentacion) {
        AccessesDataset<L64b, L64b> dataset
        {
            vector<L64b>{0,2,0,2,0,2,0,2,4,8,10,12,14},
            vector<L64b>{0,1,0,1,0,1,0,1,2,2,2,3,3}
        };

        TraceReader<L64b, L64b> reader(mcf_s);
        dataset = reader.readNextLines(10000000);// 10000000);
        // dataset = reader.readAllLines();

        BuffersDataset<int> res;
        auto b = proposedBuffersSimulator(dataset, res, 8, 6);

        PredictorSVM<MultiSVMClassifierOneToAll, int> predictor =
            PredictorSVM<MultiSVMClassifierOneToAll, int>(res, 8, 6);
        predictor.simular();

    }
    else {
        string outputName = "mcf_perlbench_omnetpp_x264.xml";
        vector<string> traceFiles = vector<string>{
            //prueba1, prueba2,
            mcf_s, perlbench_s, omnetpp_s, x264_s
        };
        vector<string> traceNames = vector<string>{
            //"pruebaCorta", "pruebaCCL",
            "mcf_s", "perlbench_s", "omnetpp_s", "x264_s"
        };

        CacheParameters cacheParams = {
            0, // Infinite cache
            0,
            8
        };

        DictionaryParameters dictParams = {
            6,
            6,
            255,
            8,
            true
        };

        PredictorParameters params = {
            cacheParams,
            dictParams
        };

        unsigned long numAccessesPerTrace = 4 * 1e6;// 1e9;
        vector<TraceInfo> tracesInfo = vector<TraceInfo>();
        for (int i = 0; i < traceNames.size(); i++) {
            tracesInfo.push_back({
                traceNames[i],
                traceFiles[i],
                numAccessesPerTrace
                });
        }

        TracePredictExperientation experimentation = TracePredictExperientation(outputName);
        experimentation.buildExperiments(tracesInfo, params, 1e6);// 10000);//
        experimentation.performExperiments();
        experimentation.exportResults();

        printf("");
    }



}

