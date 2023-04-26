// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"
#include "BuffersSimulator.h"
#include "TraceReader.h"
#include "Experimentation.h"
#include "Global.h"

// Direcciones de ficheros de trazas (distribuidas):
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


int main()
{
    // Nombre del fichero de salida:
    string outputName = "perlbench_s_distrib_buffer_svm_.xml";
    // string outputName = "cactuBSSN_s_exchange2_s_roms_s_distrib_DFCM_6_.xml";
    // string outputName = "cactuBSSN_s_exchange2_s_roms_s_distrib_BufferSVM_6_.xml";

    // Lista de ficheros de traza a ser utilizados:
    vector<string> traceFiles = vector<string>{
        // cactuBSSN_s, exchange2_s, roms_s,
        //mcf_s
        perlbench_s
        // leela_s
        // gcc_s
        // exchange2_s
    };

    // Lista de nombres de trazas: 
    vector<string> traceNames = vector<string>{
        // "cactuBSSN_s", "exchange2_s", "roms_s"
        //"mcf_s"
        "perlbench_s"
        //"cactuBSSN_s"
        // "exchange2_s"
    };

    // Parámetros de caché: (1) núm.de bits de índice, (2) num. de vías, (3) longitud de cada secuencia (historia), 
    // (4) guardar historia de entrada y clase de salida en el dataset aunque la secuencia guardada en caché no esté completa.
    // - En el caso de predictor BufferSVM, un núm.de bits de índice menor que 0 (<0) indica que la caché será de tamaño infinito.
    // - En el caso de predictor DFCMInfinito, una longitud de secuencia k > 0 indica un DFCM de grado k. 
    CacheParameters cacheParams = {
        10,// 8,//6,// 0,// 9,// 8,// 10, // Infinite cache
        4,// 8,// 8,// 4,
        6,// 4,// 8
        true,
        true
    };

    // Parámetros de diccionario: (1) núm. de clases (ergo, núm. de entradas), (2) máx. nivel de confianza, (3) núm. de saltos 
    // que da la confianza (para implementar pseudo-LFU), (4) guardar historia de entrada y clase de salida en el dataset aunque 
    // haya habido miss en el diccionario.
    DictionaryParameters dictParams = {
        4,
        255,
        8,
        true
    };

    PredictorParameters params = {
        PredictorModelType::BufferSVM, // Con el tipo de modelo de predictor indicamos si queremos el BufferSVM
                                        // o el DFCM-infinito.
        // PredictorModelType::DFCMInfinito,
        cacheParams,
        dictParams
    };

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

    TracePredictExperimentation experimentation = TracePredictExperimentation(outputName);
    experimentation.buildExperiments(tracesInfo, params, numAccessesPerExperiment);
    experimentation.performExperiments();
    experimentation.exportResults();

    printf("");


}

