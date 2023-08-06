// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"
#include "BuffersSimulator.h"
#include "TraceReader.h"
#include "Experimentation.h"
#include "Global.h"

// Direcciones de ficheros de trazas (distribuidas):
string rutaSSD = "D:\\TrazasDistribSPEC\\benchspec\\CPU\\";

string perlbench_s = rutaSSD + "600.perlbench_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string gcc_s = rutaSSD + "602.gcc_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string mcf_s = rutaSSD + "605.mcf_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string lbm_s = rutaSSD + "619.lbm_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string omnetpp_s = rutaSSD + "620.omnetpp_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string xalancbmk_s = rutaSSD + "623.xalancbmk_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string x264_s = rutaSSD + "625.x264_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string deepsjeng_s = rutaSSD + "631.deepsjeng_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string leela_s = rutaSSD + "641.leela_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";

string cactuBSSN_s = rutaSSD + "607.cactuBSSN_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string exchange2_s = rutaSSD + "648.exchange2_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";
string roms_s = rutaSSD + "654.roms_s\\run\\run_peak_refspeed_mytest-m64.0000\\pinatrace.out";


int main()
{
    string outputName;
    vector<string> traceFiles;
    vector<string> traceNames;

    // Nombre del fichero de salida:
    // outputName = "PruebaBufferSVMReal_NoActualizarDiccionarioCuandoMissInputBuffer.xml";
    // outputName = "PruebaBufferSVMReal__.xml";
    // outputName = "PruebaDFCMOrden8CambioEscrituraHistoriaDeltas_26_06.xml";
    // outputName = "PruebaDFCMCambioEscrituraSegundaTabla_30_06.xml";
    // outputName = "PruebaBufferSVMRealTotalMemoryCost.xml";
    // outputName = "PruebaDFCMHashOnHashReal.xml";
    // outputName = "PruebaSVMReal4clases___.xml";
    // outputName = "PruebaDFCMHashOnHashReal_delta_con_signo.xml";
    // outputName = "PruebaDFCMHashOnHashReal_segunda_tabla_mas_pequenna.xml";
    // outputName = "PruebaDFCMInfinito_cambio_acceso_mapa.xml";
    // outputName = "PruebaOrden8Infinito";
    // outputName = "PruebaDFCMRealSegundaTabla64Conjuntos2Vias.xml";
    //outputName = "PruebaDFCMRealPrimeraTabla1024Conjuntos6ViasSegundaTabla256Conjuntos4Vias.xml";
    // outputName = "PruebaBufferSVM1024Conjuntos6Vias6Clases.xml";
    outputName = "MultiPruebaDFCMGradoK.xml";

    // Lista de ficheros de traza a ser utilizados:
    traceFiles = vector<string>{

        perlbench_s,

        gcc_s,

        mcf_s,
        lbm_s,
        omnetpp_s,
        xalancbmk_s,
        x264_s,
        deepsjeng_s,

        leela_s,

        cactuBSSN_s,
        exchange2_s,
        roms_s,

    };

    //  traceFiles = vector<string>{ mcf_s };

    // Lista de nombres de trazas: 
    traceNames = vector<string>{

        "perlbench_s",

        "gcc_s",

        "mcf_s",
        "lbm_s",
        "omnetpp_s",
        "xalancbmk_s",
        "x264_s",
        "deepsjeng_s",

        "leela_s",

        "cactuBSSN_s",
        "exchange2_s",
        "roms_s",

    };
    // traceNames = vector<string>{ "mcf_s" };

    bool countTotalMemory = false;

    /*
    // Parámetros de caché: (1) núm.de bits de índice, (2) num. de vías, (3) longitud de cada secuencia (historia), 
    // (4) guardar historia de entrada y clase de salida en el dataset aunque la secuencia guardada en caché no esté completa.
    // - En el caso de predictor BufferSVM, un núm.de bits de índice menor que 0 (<0) indica que la caché será de tamaño infinito.
    // - En el caso de predictor DFCMInfinito, una longitud de secuencia k > 0 indica un DFCM de grado k. 
    CacheParametersDomain cacheParamsDomain = {
        vector<int>{7,8}, // 10,// 8,//6,// 0,// 9,// 8,// 10, // Infinite cache
        vector<int>{6},// 8,// 8,// 4,
        vector<int>{4,8},// 8,// 4,// 8
        vector<bool>{true}
    };


    CacheParametersDomain additionalCacheParamsDomain = {
        vector<int>{7, 8},// 8,// 8,//6,// 0,// 9,// 8,// 10, // Infinite cache
        vector<int>{2,4},// 8,// 8,// 4,
        vector<int>{-1},// 4,// 8
        vector<bool>{true}
    };

    // Parámetros de diccionario: (1) núm. de clases (ergo, núm. de entradas), (2) máx. nivel de confianza, (3) núm. de saltos 
    // que da la confianza (para implementar pseudo-LFU), (4) guardar historia de entrada y clase de salida en el dataset aunque 
    // haya habido miss en el diccionario.
    DictionaryParametersDomain dictParamsDomain = {
        vector < int>{6},
        vector < int>{255},
        vector < int>{8},
        vector<bool>{true}
    };
    */
    // Parámetros de caché: (1) núm.de bits de índice, (2) num. de vías, (3) longitud de cada secuencia (historia), 
    // (4) guardar historia de entrada y clase de salida en el dataset aunque la secuencia guardada en caché no esté completa.
    // - En el caso de predictor BufferSVM, un núm.de bits de índice menor que 0 (<0) indica que la caché será de tamaño infinito.
    // - En el caso de predictor DFCMInfinito, una longitud de secuencia k > 0 indica un DFCM de grado k. 
    CacheParametersDomain cacheParamsDomain = {
        vector<int>{7,8}, // 10,// 8,//6,// 0,// 9,// 8,// 10, // Infinite cache
        vector<int>{6},// 8,// 8,// 4,
        vector<int>{8},// 8,// 4,// 8
        vector<bool>{true}
    };


    CacheParametersDomain additionalCacheParamsDomain = {
        vector<int>{7},// 8,// 8,//6,// 0,// 9,// 8,// 10, // Infinite cache
        vector<int>{4},// 8,// 8,// 4,
        vector<int>{-1},// 4,// 8
        vector<bool>{true}
    };

    // Parámetros de diccionario: (1) núm. de clases (ergo, núm. de entradas), (2) máx. nivel de confianza, (3) núm. de saltos 
    // que da la confianza (para implementar pseudo-LFU), (4) guardar historia de entrada y clase de salida en el dataset aunque 
    // haya habido miss en el diccionario.
    DictionaryParametersDomain dictParamsDomain = {
        vector < int>{6},
        vector < int>{255},
        vector < int>{8},
        vector<bool>{true}
    };


    PredictorParametersDomain paramsDomain = {
        // PredictorModelType::BufferSVM, // Con el tipo de modelo de predictor indicamos si queremos el BufferSVM
        // o el DFCM-infinito.
        vector<PredictorModelType>{PredictorModelType::DFCM},
        cacheParamsDomain,
        additionalCacheParamsDomain,
        dictParamsDomain
    };

    unsigned long numAccessesPerTrace = 1e9;
    // unsigned long numAccessesPerTrace = 1e7;
    unsigned long numAccessesPerExperiment = 1e6;
    vector<TraceInfo> tracesInfo = vector<TraceInfo>();
    for (int i = 0; i < traceNames.size(); i++) {
        tracesInfo.push_back({
            traceNames[i],
            traceFiles[i],
            numAccessesPerTrace
            });
    }
    /*
    TracePredictExperimentation experimentation = TracePredictExperimentation(outputName, countTotalMemory);
    // experimentation.buildExperiments(tracesInfo, params, numAccessesPerExperiment);
    experimentation.buildExperiments(tracesInfo, paramsDomain, numAccessesPerExperiment);
    experimentation.performExperiments();
    experimentation.exportResults();
    */
    vector<TracePredictExperimentation> experimentations =
        TracePredictExperimentation::createAndBuildExperimentations(tracesInfo, paramsDomain, numAccessesPerExperiment, outputName, countTotalMemory);
    for (auto& experimentation : experimentations) {
        experimentation.performExperiments();
        experimentation.exportResults();
    }

    printf("");


}

