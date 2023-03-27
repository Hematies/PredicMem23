// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"
#include "BuffersSimulator.h"
#include "TraceReader.h"
#include "Experimentation.h"
#include "Global.h"

// Direcciones de ficheros de trazas (distribuidas):
string perlbench_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/600.perlbench_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string gcc_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/602.gcc_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string mcf_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/605.mcf_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string lbm_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/619.lbm_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string omnetpp_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/620.omnetpp_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string xalancbmk_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/623.xalancbmk_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string x264_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/625.x264_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string deepsjeng_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/631.deepsjeng_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string leela_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/641.leela_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";

string cactuBSSN_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/607.cactuBSSN_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string exchange2_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/648.exchange2_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";
string roms_s = "/home/arios/Projects/PredicMem23/trazas/TrazasDistribSPEC/benchspec/CPU/654.roms_s/run/run_peak_refspeed_mytest-m64.0000/pinatrace.out";


int main()
{
    // Nombre del fichero de salida:
    //string outputName = "perlbench_s_distrib_buffer_svm_.xml";
    // string outputName = "cactuBSSN_s_exchange2_s_roms_s_distrib_DFCM_6_.xml";
    // string outputName = "cactuBSSN_s_exchange2_s_roms_s_distrib_BufferSVM_6_.xml";

    // Lista de ficheros de traza a ser utilizados:
    vector<string> traceFiles = vector<string>{
        // cactuBSSN_s, exchange2_s, roms_s,
        cactuBSSN_s,
        mcf_s,
        perlbench_s
        // leela_s
        // gcc_s
        // exchange2_s
    };

    // Lista de nombres de trazas: 
    vector<string> traceNames = vector<string>{
        // "cactuBSSN_s", "exchange2_s", "roms_s"
        "mcf_s",
        "perlbench_s",
        "cactuBSSN_s"
        // "exchange2_s"
    };

    // // Parámetros de caché: (1) núm.de bits de índice, (2) num. de vías, (3) longitud de cada secuencia (historia), 
    // // (4) guardar historia de entrada y clase de salida en el dataset aunque la secuencia guardada en caché no esté completa.
    // // - En el caso de predictor BufferSVM, un núm.de bits de índice menor que 0 (<0) indica que la caché será de tamaño infinito.
    // // - En el caso de predictor DFCMInfinito, una longitud de secuencia k > 0 indica un DFCM de grado k. 
    // CacheParameters cacheParams = {
    //     10,// 8,//6,// 0,// 9,// 8,// 10, // Infinite cache
    //     4,// 8,// 8,// 4,
    //     6,// 4,// 8
    //     true
    // };

    // // Parámetros de diccionario: (1) núm. de clases (ergo, núm. de entradas), (2) máx. nivel de confianza, (3) núm. de saltos 
    // // que da la confianza (para implementar pseudo-LFU), (4) guardar historia de entrada y clase de salida en el dataset aunque 
    // // haya habido miss en el diccionario.
    // DictionaryParameters dictParams = {
    //     4,
    //     255,
    //     8,
    //     true
    // };

    unsigned long numAccessesPerTrace = 1e9;
    unsigned long numAccessesPerExperiment = 1e6;
    vector<TraceInfo> tracesInfo = vector<TraceInfo>();
    for (int i = 0; i < traceNames.size(); i++) {
        tracesInfo.push_back({
            traceNames[i],
            traceFiles[i],
            numAccessesPerTrace
            });
    }


    // Bucle para hacer los tests Propuestos por Pable desde el index 9 al 16, ambos inclusive
    for(int test_idx=9; test_idx<=16; test_idx++){
        string outputName;
        CacheParameters cacheParams;
        DictionaryParameters dictParams;
        switch (test_idx)
        {
        case 9:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_9.xml";
            cacheParams = {10, 2, 8, true};
            dictParams = {4, 255, 8, true};
            break;

        case 10:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_10.xml";
            cacheParams = {7, 4, 8, true};
            dictParams = {4, 255, 8, true};
            break;

        case 11:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_11.xml";
            cacheParams = {8, 4, 8, true};
            dictParams = {4, 255, 8, true};
            break;

        case 12:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_12.xml";
            cacheParams = {10, 4, 8, true};
            dictParams = {4, 255, 8, true};
            break;

        case 13:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_13.xml";
            cacheParams = {7, 2, 4, true};
            dictParams = {8, 255, 8, true};
            break;

        case 14:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_14.xml";
            cacheParams = {8, 2, 4, true};
            dictParams = {8, 255, 8, true};
            break;

        case 15:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_15.xml";
            cacheParams = {10, 2, 4, true};
            dictParams = {8, 255, 8, true};
            break;

        case 16:
            outputName = "mcf_s-perlbench_s-cactuBSSN_s-distrib_buffer_svm_test_16.xml";
            cacheParams = {7, 4, 4, true};
            dictParams = {8, 255, 8, true};
            break;
        
        default:
            break;
        }

        PredictorParameters params = {
            PredictorModelType::BufferSVM, // Con el tipo de modelo de predictor indicamos si queremos el BufferSVM
                                            // o el DFCM-infinito.
            // PredictorModelType::DFCMInfinito,
            cacheParams,
            dictParams
        };

        TracePredictExperimentation experimentation = TracePredictExperimentation(outputName);
        experimentation.buildExperiments(tracesInfo, params, numAccessesPerExperiment);
        experimentation.performExperiments();
        experimentation.exportResults();

        printf("");

    }
}

