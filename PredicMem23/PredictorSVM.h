#pragma once
#include <string>
#include <iostream>
#include "SVMClassifier.hpp"
#include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"
#include "PredictorModel.h"

using namespace std;


// extern struct PredictResults;
extern struct BuffersSVMPredictResultsAndCosts;

template<typename T_pred, typename T_entrada>
class PredictorSVM : PredictorModel<L64bu, T_entrada>
{

private:
	int numPartesMostrar = 10000;
public:
	vector<vector<float>> datosEntrada = vector<vector<float>>();
	vector<char> datosSalida = vector<char>();
	vector<char> mascaraErroresBufferes = vector<char>();
	vector<char> mascaraErroresCache = vector<char>();
	vector<char> mascaraErroresDiccionario = vector<char>();

	long numAciertos = 0;
	int numMuestrasLote = 1;
	int numRepeticiones = 1;
	double tasaExito = 0.0;
	T_pred modelo;

	int numElemSecuencia = 0;
	int numClases = 0;

	~PredictorSVM() {
		datosEntrada.clear();
		datosSalida.clear();
		mascaraErroresBufferes.clear();
		mascaraErroresCache.clear();
		mascaraErroresDiccionario.clear();
	}

	PredictorSVM(BuffersDataset<T_entrada> datasetClases, int numElemSecuencia, int numClases) {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Clase no es subtipo de MultiSVMClassifier");

		this->numElemSecuencia = numElemSecuencia;
		this->numClases = numClases;
		importarDatos(datasetClases);
		inicializarModelo();
	}

	PredictorSVM(int numElemSecuencia, int numClases) {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Clase no es subtipo de MultiSVMClassifier");

		this->numElemSecuencia = numElemSecuencia;
		this->numClases = numClases;
		inicializarModelo();
	}

	PredictorSVM() {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Clase no es subtipo de MultiSVMClassifier");

		this->numElemSecuencia = 0;
		this->numClases = 0;
	}

	void importarDatos(AccessesDataset<L64bu, L64bu>& datos, BuffersDataset<T_entrada>& datasetClases) {
		importarDatos(datasetClases);
	}

	void importarDatos(BuffersDataset<T_entrada>& datasetClases) {
		for (int i = 0; i < datasetClases.inputAccesses.size(); i++) {
			vector<float> entrada = vector<float>();
			char salida = -1;
			char haHabidoErrorBufferes = false;

			for (int j = 0; j < datasetClases.inputAccesses[i].size(); j++) {
				entrada.push_back(((float)datasetClases.inputAccesses[i][j]) / numElemSecuencia + 1.0);
			}

			salida = datasetClases.outputAccesses[i];
			haHabidoErrorBufferes = !datasetClases.isValid[i];

			this->datosEntrada.push_back(entrada);
			this->datosSalida.push_back(salida);
			this->mascaraErroresBufferes.push_back(haHabidoErrorBufferes);
			this->mascaraErroresCache.push_back(datasetClases.isCacheMiss[i]);
			this->mascaraErroresDiccionario.push_back(datasetClases.isDictionaryMiss[i]);
		}
	}

	void inicializarModelo() {
		// double c = 0.5;
		double c = 1.0;
		double learningRate = 0.5;
		// double c = 0.8;
		modelo = T_pred(this->numElemSecuencia, this->numClases, c, 1, learningRate);
		// ajustarPredictor(std::vector<float>(datosEntrada[0].size(), 0.0), 0);
	}

	void ajustarPredictor(vector<float> entrada, int salida) {
		auto in = vector<vector<double>>{ vector<double>(entrada.begin(), entrada.end()) };
		auto out = vector<int>{ salida };
		this->modelo.fit(in, out);
	}

	int predecir(vector<float> entrada) {
		auto in = vector<vector<double>>{ vector<double>(entrada.begin(), entrada.end()) };
		return this->modelo.predict(in)[0];
	}

	shared_ptr<PredictResultsAndCosts> simular(bool inicializar = true) {

		BuffersSVMPredictResultsAndCosts resultsAndCosts = BuffersSVMPredictResultsAndCosts();
		double numDictionaryMisses = 0.0;
		double numCacheMisses = 0.0;


		if (inicializar) {
			this->inicializarModelo();

		}

		numAciertos = 0;
		tasaExito = 0.0;

		for (int i = 0; i < datosEntrada.size(); i++) {
			vector<float> entrada = vector<float>(datosEntrada[i].begin(), datosEntrada[i].end());
			int salida = datosSalida[i];
			auto haHabidoErrorBufferes = mascaraErroresBufferes[i];
			auto haHabidoErrorCache = mascaraErroresCache[i];
			auto haHabidoErrorDiccionario = mascaraErroresDiccionario[i];

			int salidaPredicha = -1;
			if(!haHabidoErrorBufferes)
				salidaPredicha = predecir(entrada);

			bool haHabidoFalloPrediccion = (salida != salidaPredicha);
			bool haHabidoFallo = haHabidoFalloPrediccion || haHabidoErrorBufferes;

			// Si ha habido un fallo, entrenamos con la muestra de entrada y salida:
			if (haHabidoFalloPrediccion) {
				ajustarPredictor(entrada, salida);
			}
			else if (!haHabidoErrorBufferes)
				numAciertos++;
			else {
				if (haHabidoErrorDiccionario) numDictionaryMisses++;
				if (haHabidoErrorCache) numCacheMisses++;
			}
				

			if (i % numPartesMostrar == 0) {
			// 
			// if (!haHabidoErrorBufferes){
				string in = "";
				for (auto e : entrada)
					in += to_string((e - 1.0) * numElemSecuencia) + ", ";
				std::cout << in << " -> " << salida << " vs " << salidaPredicha << std::endl;
				std::cout << "Tasa de éxito: " << (double)numAciertos / (i + 1) << " ; " << ((double)i) / datosEntrada.size() << std::endl;
			}

		}

		tasaExito = ((double)numAciertos) / datosEntrada.size();

		resultsAndCosts.hitRate = tasaExito;
		resultsAndCosts.dictionaryMissRate = numDictionaryMisses / datosEntrada.size();
		resultsAndCosts.cacheMissRate = numCacheMisses / datosEntrada.size();
		resultsAndCosts.modelMemoryCost = getModelMemoryCosts();
		return shared_ptr<PredictResultsAndCosts>((PredictResultsAndCosts*) new BuffersSVMPredictResultsAndCosts(resultsAndCosts));
	}

	double getModelMemoryCosts() {
		int numSVMs = this->modelo.SVMsTable.size();
		int numElements = this->modelo.numFeatures + 1;

		return numElements * numSVMs; // For now, we just return the total number of elements (weights) of the model.
	}
};


