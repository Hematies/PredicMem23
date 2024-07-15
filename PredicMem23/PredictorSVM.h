/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//    Copyright (c) 2024  Pablo Sánchez Cuevas                    //
//                                                                             //
//    This file is part of PredicMem23.                                            //
//                                                                             //
//    PredicMem23 is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by     //
//    the Free Software Foundation, either version 3 of the License, or        //
//    (at your option) any later version.                                      //
//                                                                             //
//    PredicMem23 is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the              //
//    GNU General Public License for more details.                             //
//                                                                             //
//    You should have received a copy of the GNU General Public License        //
//    along with PredicMem23. If not, see <
// http://www.gnu.org/licenses/>.
//
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <iostream>
#include "SVMClassifier.hpp"
#include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"
#include "PredictorModel.h"

using namespace std;



template<typename T_pred, typename T_entrada>
class PredictorSVM : PredictorModel<L64bu, T_entrada>
{

private:
	int numPartesMostrar = 10000;
	int numClasesEntrada;
public:
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
	
	~PredictorSVM() {
		clean();
	}
	
	void clean() {
		/*
		datosEntrada.clear();
		datosSalida.clear();
		mascaraEntradasPredecibles.clear();
		mascaraErroresCache.clear();
		mascaraErroresDiccionario.clear();
		*/
		this->datosEntrada = vector<vector<float>>();
		this->datosSalida = vector<char>();
		this->mascaraEntradasPredecibles = vector<char>();
		this->mascaraErroresCache = vector<char>();
		this->mascaraErroresDiccionario = vector<char>();
	}

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

	void importarDatos(AccessesDataset<L64bu, L64bu>& datos, BuffersDataset<T_entrada>& datasetClases) {
		importarDatos(datasetClases);
	}

	void importarDatos(BuffersDataset<T_entrada>& datasetClases) {
		for (int i = 0; i < datasetClases.inputAccesses.size(); i++) {
			vector<float> entrada = vector<float>();
			char salida = -1;
			char esEntradaValida = false;

			for (int j = 0; j < datasetClases.inputAccesses[i].size(); j++) {
				entrada.push_back(((float)datasetClases.inputAccesses[i][j]) / numClasesEntrada + 1.0);
			}

			salida = datasetClases.outputAccesses[i];
			esEntradaValida = datasetClases.isValid[i];

			this->datosEntrada.push_back(entrada);
			this->datosSalida.push_back(salida);
			this->mascaraEntradasPredecibles.push_back(esEntradaValida);
			this->mascaraErroresCache.push_back(datasetClases.isCacheMiss[i]);
			this->mascaraErroresDiccionario.push_back(datasetClases.isDictionaryMiss[i]);
		}
	}

	void inicializarModelo() {
		// double c = 0.5;
		double c = 1.0;
		double learningRate = 0.7;
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
			auto esEntradaPredecible = mascaraEntradasPredecibles[i];
			auto haHabidoErrorCache = mascaraErroresCache[i];
			auto haHabidoErrorDiccionario = mascaraErroresDiccionario[i];

			int salidaPredicha = -1;
			if(esEntradaPredecible)
				salidaPredicha = predecir(entrada);

			bool haHabidoFalloPrediccion = (salida != salidaPredicha);

			// Si ha habido un fallo, entrenamos con la muestra de entrada y salida:
			// bool hayQueAjustar = (haHabidoFalloPrediccion && esEntradaPredecible) || (haHabidoErrorDiccionario && !esEntradaPredecible);
			bool hayQueAjustar = !haHabidoErrorCache && (haHabidoFalloPrediccion || haHabidoErrorDiccionario);
			if (hayQueAjustar) {
				ajustarPredictor(entrada, salida);
			}
			
			bool hayHit = esEntradaPredecible && !haHabidoErrorDiccionario && !haHabidoErrorCache && !haHabidoFalloPrediccion;
			if (hayHit)
				numAciertos++;

			if (haHabidoErrorDiccionario) numDictionaryMisses++;
			if (haHabidoErrorCache) numCacheMisses++;
				

			if (i % numPartesMostrar == 0) {
			// 
			// if (!esEntradaPredecible){
				string in = "";
				for (auto e : entrada)
					in += to_string((e - 1.0) * numClasesEntrada) + ", ";
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

		return numElements * sizeof(float) * numSVMs; 
			// For now, we return the total number bytes of the elements (weights) of the model.
	}
};


