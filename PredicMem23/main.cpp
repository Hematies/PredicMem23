// PredictorSVM.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "PredictorSVM.h"

// string nombreFicheroDatos = "foto.jpg";//  "datasetClases.bmp"; // // "..\\..\\datos\\datasetClases.bmp";
// string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\datasetClases2.bmp";
string nombreFicheroDatos = "C:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\omnetpp_s_0.bmp";
//string nombreFicheroDatos = "A:\\Users\\pablo\\Desktop\\Doctorado\\PredicMem22\\PredictorSVM\\datos\\mcf_s_0.bmp";

int main()
{
    PredictorSVM<MultiSVMClassifierOneToAll> predictor = PredictorSVM<MultiSVMClassifierOneToAll>(nombreFicheroDatos, 8, 6);
    predictor.simular();
}

