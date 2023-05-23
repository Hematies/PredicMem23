#pragma once
#include <string>
#include <iostream>
#include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"
#include "PredictorModel.h"
#include "PredictorDFCM.h"

using namespace std;


template<typename T, typename Delta>
class PredictorDFCMInfinitoGradoK : public PredictorModel<T, int>
{
protected:
	int numPartesMostrar = 10000;

	AccessesDataset<T, T> datos;

	bool accederTablaInstrHash(T instruccion, T* ultimoAcceso, vector<Delta>& deltas, T* hash) {
		if (this->tablaInstrHash.find(instruccion) == this->tablaInstrHash.end()) {
			return false;
		}
		else {
			auto tupla = this->tablaInstrHash[instruccion];
			*ultimoAcceso = get<0>(tupla);
			deltas = vector<Delta>(get<1>(tupla));
			*hash = 0;
			for (Delta delta : deltas)
				*hash = *hash ^ delta;
			return true;
		}
	}


	bool accederTablaHashDelta(T hash, Delta* delta) {
		if (tablaHashDelta.find(hash) == tablaHashDelta.end()) {
			return false;
		}
		else {
			*delta = tablaHashDelta[hash];
			return true;
		}
	}

	bool escribirTablaInstrHash(T instruccion, T ultimoAcceso, Delta nuevoDelta) {
		T h;
		vector<Delta> deltas = vector<Delta>(this->numUltimosDeltas, 0);
		bool estabaEnTabla = accederTablaInstrHash(instruccion, &ultimoAcceso, deltas, &h);
		if (estabaEnTabla) {
			for (int i = 0; i < deltas.size() - 1; i++)
				deltas[i] = deltas[i - 1];
		}
		deltas[deltas.size() - 1] = nuevoDelta;
		tablaInstrHash[instruccion] = tuple<T, vector<Delta>>(ultimoAcceso, deltas);
		return estabaEnTabla;
	}


	bool escribirTablaHashDelta(T hash, Delta delta) {
		Delta d;
		bool estabaEnTabla = accederTablaHashDelta(hash, &d);
		tablaHashDelta[hash] = delta;
		return estabaEnTabla;
	}

public:


	map<T, tuple<T, vector<Delta>>> tablaInstrHash;
	map<T, Delta> tablaHashDelta;

	int numUltimosDeltas = 0;


	PredictorDFCMInfinitoGradoK(AccessesDataset<T, T>& datos, int numUltimosDeltas) {
		this->datos = datos;
		this->numUltimosDeltas = numUltimosDeltas;
		this->inicializarPredictor();
	}

	PredictorDFCMInfinitoGradoK(int numUltimosDeltas) {
		this->numUltimosDeltas = numUltimosDeltas;
		this->inicializarPredictor();
	}


	~PredictorDFCMInfinitoGradoK() {
		this->clean();
	}

	void clean() {
		/*
		this->tablaInstrHash.clear();
		this->tablaHashDelta.clear();
		*/
		this->tablaInstrHash = {};
		this->tablaHashDelta = {};
		this->datos = {};
	}

	void importarDatos(AccessesDataset<T, T>& datos, BuffersDataset<int>& datasetClases) {
		this->datos = datos;
	}

	void inicializarPredictor() {
		tablaInstrHash = {};
		tablaHashDelta = {};
	}


	shared_ptr<PredictResultsAndCosts> simular(bool inicializar = true) {

		DFCMPredictResultsAndCosts resultsAndCosts = DFCMPredictResultsAndCosts();
		double numFirstTableMisses = 0.0;
		double numSecondTableMisses = 0.0;


		if (inicializar) {
			this->inicializarPredictor();

		}

		long numAciertos = 0;
		double tasaExito = 0.0;

		for (int i = 0; i < datos.accesses.size(); i++) {
			T entrada = datos.accessesInstructions[i];
			T salida = datos.accesses[i];
			T salidaPredicha;
			bool instrEnTabla, hashEnTabla;
			bool haHabidoFalloTablas = !predecir(entrada, &salidaPredicha, &instrEnTabla, &hashEnTabla);

			bool haHabidoFalloPrediccion = (salida != salidaPredicha);
			bool haHabidoFallo = haHabidoFalloPrediccion || haHabidoFalloTablas;

			// Si ha habido un fallo, entrenamos con la muestra de entrada y salida:
			if (haHabidoFallo) {
				if (haHabidoFalloTablas) {
					if (!instrEnTabla)
						numFirstTableMisses++;
					if (!hashEnTabla)
						numSecondTableMisses++;
				}
			}
			else
				numAciertos++;

			ajustarPredictor(entrada, salida);


			if (i % numPartesMostrar == 0) {
				// 
				// if (!haHabidoErrorBufferes){
				std::cout << entrada << " -> " << salida << " vs " << salidaPredicha << std::endl;
				std::cout << "Tasa de éxito: " << (double)numAciertos / (i + 1) << " ; " << ((double)i) / datos.accesses.size() << std::endl;
			}

		}

		tasaExito = ((double)numAciertos) / datos.accesses.size();

		resultsAndCosts.hitRate = tasaExito;
		resultsAndCosts.firstTableMissRate = numFirstTableMisses / datos.accesses.size();
		resultsAndCosts.secondTableMissRate = numSecondTableMisses / datos.accesses.size();
		double firstTableCost, secondTableCost;
		resultsAndCosts.totalMemoryCost = getMemoryCosts(&firstTableCost, &secondTableCost);
		resultsAndCosts.firstTableMemoryCost = firstTableCost;
		resultsAndCosts.secondTableMemoryCost = secondTableCost;
		return shared_ptr<PredictResultsAndCosts>((PredictResultsAndCosts*)new DFCMPredictResultsAndCosts(resultsAndCosts));
	}


	void ajustarPredictor(T instruccion, T acceso) {
		// Primera tabla
		T hash;
		T accesoAnterior;
		Delta delta;
		vector<Delta> ultimosDeltas;
		bool hashEnTabla = accederTablaInstrHash(instruccion, &accesoAnterior, ultimosDeltas, &hash);
		if (!hashEnTabla) {
			accesoAnterior = acceso;
			hash = 0;
			delta = 0;
			escribirTablaInstrHash(instruccion, acceso, delta);
			escribirTablaHashDelta(hash, delta);
		}
		else {
			delta = acceso - accesoAnterior;
			escribirTablaInstrHash(instruccion, acceso, delta);
			escribirTablaHashDelta(hash, delta);
		}

	}

	bool predecir(T instruccion, T* acceso, bool* instrEnTabla, bool* hashEnTabla) {
		T hash;
		T ultimoAcceso;
		*instrEnTabla = false;
		*hashEnTabla = false;
		vector<Delta> deltas;

		*instrEnTabla = accederTablaInstrHash(instruccion, &ultimoAcceso, deltas, &hash);
		if (!(*instrEnTabla)) return false;
		else {
			Delta delta;
			*hashEnTabla = this->accederTablaHashDelta(hash, &delta);
			if (!(*hashEnTabla))
				return false;
			else *acceso =
				ultimoAcceso + delta;
		}
		return true;
	}


	double getMemoryCosts(double* firstTableCost, double* secondTableCost) {
		*firstTableCost = (2 + this->numUltimosDeltas) * sizeof(T) * this->tablaInstrHash.size();
		*secondTableCost = 2 * sizeof(T) * this->tablaHashDelta.size();
		return *firstTableCost + *secondTableCost;
	}
};


