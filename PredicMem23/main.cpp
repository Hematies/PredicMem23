// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"
#include "BuffersSimulator.h"
#include "TraceReader.h"
#include "Experimentation.h"
#include "Global.h"

string perlbench_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\600.perlbench_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string gcc_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\602.gcc_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string mcf_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\605.mcf_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string lbm_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\619.lbm_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string omnetpp_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\620.omnetpp_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string xalancbmk_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\623.xalancbmk_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string x264_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\625.x264_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string deepsjeng_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\631.deepsjeng_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string leela_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\641.leela_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";

string cactuBSSN_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\607.cactuBSSN_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string exchange2_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\648.exchange2_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string roms_s = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\654.roms_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";


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

        BuffersDataset<int> res;
        auto b = proposedBuffersSimulator(dataset, res, cacheParams, dictParams);

        PredictorSVM<MultiSVMClassifierOneToAll, int> predictor =
            PredictorSVM<MultiSVMClassifierOneToAll, int>(res, 8, 6);
        predictor.simular();

    }
    else {
        /*
        string outputName = "perlbench_gcc_mcf_lbm_omnetpp_xalancbmk_x264_deepsjeng_leela_distrib.xml";
        vector<string> traceFiles = vector<string>{
            //prueba1, prueba2,
            perlbench_s,gcc_s,mcf_s,lbm_s,omnetpp_s,xalancbmk_s,x264_s,deepsjeng_s,leela_s
            //deepsjeng_s
        };
        vector<string> traceNames = vector<string>{
            //"pruebaCorta", "pruebaCCL",
            "perlbench_s","gcc_s","mcf_s","lbm_s","omnetpp_s","xalancbmk_s","x264_s","deepsjeng_s",
            "leela_s"
            //"deepsjeng_s"
        };
        */
        string outputName = "mcf_s_distrib_real_cache.xml";
        vector<string> traceFiles = vector<string>{
            //prueba1, prueba2,
            //cactuBSSN_s, exchange2_s, roms_s
            mcf_s
        };
        vector<string> traceNames = vector<string>{
            //"pruebaCorta", "pruebaCCL",
            //"cactuBSSN_s", "exchange2_s", "roms_s"
            "mcf_s"
        };

        CacheParameters cacheParams = {
            10, // Infinite cache
            4,
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

        unsigned long numAccessesPerTrace = 1e9;
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

