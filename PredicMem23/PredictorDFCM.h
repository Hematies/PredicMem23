#pragma once
#include <string>
#include <iostream>
#include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"

using namespace std;


// extern struct PredictResults;
extern struct PredictResultsAndCosts;

template<typename T>
class PredictorDFCMInfinito
{
private:
	int numPartesMostrar = 10000;

	AccessesDataset<T,T> datos;

	bool accederTablaInstrHash(T instruccion, T* ultimoAcceso, T* hash) {
		if (tablaInstrHash.find(instruccion) == tablaInstrHash.end()) {
			return false;
		}
		else {
			auto tupla = tablaInstrHash[instruccion];
			*ultimoAcceso = tupla.get<0>();
			*hash = tupla.get<1>();
			return true;
		}
	}

	bool accederTablaHashDelta(T hash, T* delta) {
		if (tablaHashDelta.find(hash) == tablaHashDelta.end()) {
			return false;
		}
		else {
			*delta = tablaHashDelta[hash];
			return true;
		}
	}

	bool escribirTablaInstrHash(T instruccion, T ultimoAcceso, T hash) {
		bool estabaEnTabla = accederTablaInstrHash(instruccion, &ultimoAcceso, &hash);
		tablaInstrHash[instruccion] = tuple<T,T>(ultimoAcceso, hash);
		return estabaEnTabla;
	}

	bool escribirTablaHashDelta(T hash, T delta) {
		bool estabaEnTabla = accederTablaHashDelta(hash, &delta);
		tablaHashDelta[hash] = delta;
		return estabaEnTabla;
	}

	T calcularHash(T v1, T v2) {
		return v1 ^ v2;
	}

public:
	
	long numAciertos = 0;

	double tasaExito = 0.0;

	map<T, tuple<T,T>> tablaInstrHash;
	map<T, T> tablaHashDelta;

	PredictorDFCMInfinito(AccessesDataset<T,T>& datos) {
		this->datos = datos;
		inicializarPredictor();
	}

	PredictorDFCMInfinito() {
		inicializarPredictor();
	}

	void inicializarPredictor() {
		tablaInstrHash = map<T, T>();
		tablaHashDelta = map<T, T>();
	}


	void ajustarPredictor(T instruccion, T acceso) {
		// Primera tabla
		T hash;
		T accesoAnterior;
		T delta;
		bool hashEnTabla = accederTablaInstrHash(instruccion, &accesoAnterior, &hash);
		if (!hashEnTabla) {
			accesoAnterior = acceso;
			hash = 0;
			delta = 0;
			escribirTablaInstrHash(instruccion, accesoAnterior, hash);
			escribirTablaHashDelta(hash, delta);
		}
		else {
			delta = acceso - accesoAnterior;
			hash = hash ^ delta;
			escribirTablaInstrHash(instruccion, acceso, hash);
			escribirTablaHashDelta(hash, delta);
		}
		
	}

	bool predecir(T instruccion, T* acceso, bool* instrEnTabla, bool* hashEnTabla) {
		T hash;
		T ultimoAcceso;
		*instrEnTabla = false;
		*hashEnTabla = false;

		*instrEnTabla = accederTablaInstrHash(instruccion, &ultimoAcceso, &hash);
		if (!(*instrEnTabla)) return false;
		else {
			T delta;
			*hashEnTabla = accederTablaHashDelta(hash, &delta);
			if (!(*hashEnTabla)) return false;
			else *acceso = ultimoAcceso + delta;
		}
		return true;
	}

	bool predecir(T instruccion, T* acceso) {
		bool a, b;
		return prededir(instruccion, acceso, &a, &b);
	}

	DFCMPredictResultsAndCosts simular(bool inicializar = true) {

		struct DFCMPredictResultsAndCosts resultsAndCosts;
		double numFirstTableMisses = 0.0;
		double numSecondTableMisses = 0.0;


		if (inicializar) {
			this->inicializarPredictor();

		}

		numAciertos = 0;
		tasaExito = 0.0;

		for (int i = 0; i < datos.accesses.size(); i++) {
			T entrada = datos.accessesInstructions[i];
			T salida = datos.accesses[i];
			T salidaPredicha;
			bool instrEnTabla, hashEnTabla;
			bool haHabidoFalloTablas = predecir(entrada, &salidaPredicha, &instrEnTabla, &hashEnTabla);

			bool haHabidoFalloPrediccion = (salida != salidaPredicha);
			bool haHabidoFallo = haHabidoFalloPrediccion || haHabidoFalloTablas;

			// Si ha habido un fallo, entrenamos con la muestra de entrada y salida:
			if (haHabidoFallo) {
				ajustarPredictor(entrada, salida);

				if(haHabidoFalloTablas) {
					if (!instrEnTabla) numFirstTableMisses++;
					if (!hashEnTabla) numSecondTableMisses++;
				}
			}
			else
				numAciertos++;
			

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
		return resultsAndCosts;
	}

	double getMemoryCosts(double* firstTableCost, double* secondTableCost) {
		*firstTableCost = 3 * sizeof(T) * tablaInstrHash.size();
		*secondTableCost = 2 * sizeof(T) * tablaHashDelta.size();
		return *firstTableCost + *secondTableCost;
	}
};


