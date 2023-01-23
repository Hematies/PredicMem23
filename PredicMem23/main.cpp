// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"
#include "BuffersSimulator.h"

// string nombreFicheroDatos = "foto.jpg";//  "datasetClases.bmp"; // // "..\\..\\datos\\datasetClases.bmp";
// string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\datasetClases2.bmp";
string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\omnetpp_s_0.bmp";
//string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\mcf_s_0.bmp";

/*
extern BuffersSimulator<long, long, int, long>
proposedBuffersSimulator(AccessesDataset<long, long>& dataset, BuffersDataset<int>& classesDataset,
    int numHistoryAccesses, int numClasses,
    int maxConfidence, int numConfidenceJumps);
    */

int main()
{
    
    AccessesDataset<long, long> dataset
    {
        vector<long>{0,0,0,0,0,0,0,0},
        vector<long>{0,1,0,1,0,1,0,1}
    };
    BuffersDataset<int> res;
    auto b = proposedBuffersSimulator(dataset, res, 2, 2);
    


    PredictorSVM<MultiSVMClassifierOneToAll> predictor = PredictorSVM<MultiSVMClassifierOneToAll>(nombreFicheroDatos, 8, 6);
    predictor.simular();
}

