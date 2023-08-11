#pragma once
#include <string>
#include <iostream>
#include "SVMClassifier.hpp"
// #include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"
#include "PredictorModel.h"

using namespace std;



template<typename T_pred, typename T_entrada>
class PredictorSVM : PredictorModel<L64bu, T_entrada>
{
	
public:
	int numPartesMostrar = 10000;
	int numClasesEntrada;
	vector<vector<float>> datosEntrada = vector<vector<float>>();
	vector<char> datosSalida = vector<char>();
	vector<char> mascaraEntradasPredecibles = vector<char>();
	vector<char> mascaraErroresCache = vector<char>();
	vector<char> mascaraErroresDiccionario = vector<char>();

	long numAciertos = 0;
	int numMuestrasLote = 1;
	int numRepeticiones = 1;
	double tasaExito = 0.0;
	T_pred modelo;

	int numElemSecuencia = 0;
	int numClases = 0;

	bool predictOnNonValidInput;
	
	~PredictorSVM();
	void clean();
	PredictorSVM(BuffersDataset<T_entrada> datasetClases, int numElemSecuencia, int numClases, bool predictOnNonValidInput) {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Clase no es subtipo de MultiSVMClassifier");

		this->numElemSecuencia = numElemSecuencia;
		this->numClases = numClases;
		this->predictOnNonValidInput = predictOnNonValidInput;
		if (predictOnNonValidInput) numClasesEntrada++;


		importarDatos(datasetClases);
		inicializarModelo();
	}
	PredictorSVM(int numElemSecuencia, int numClases, bool predictOnNonValidInput) {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Clase no es subtipo de MultiSVMClassifier");

		this->numElemSecuencia = numElemSecuencia;
		this->numClases = numClases;
		this->predictOnNonValidInput = predictOnNonValidInput;
		if (predictOnNonValidInput) numClasesEntrada++;


		inicializarModelo();
	}
	PredictorSVM() {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Clase no es subtipo de MultiSVMClassifier");

		this->numElemSecuencia = 0;
		this->numClases = 0;
		this->predictOnNonValidInput = true;

	}
	void importarDatos(AccessesDataset<L64bu, L64bu>& datos, BuffersDataset<T_entrada>& datasetClases);
	void importarDatos(BuffersDataset<T_entrada>& datasetClases);
	void inicializarModelo();
	void ajustarPredictor(vector<float> entrada, int salida);
	int predecir(vector<float> entrada);
	shared_ptr<PredictResultsAndCosts> simular(bool inicializar = true);
	double getModelMemoryCosts();
};


