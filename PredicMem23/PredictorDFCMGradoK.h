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
class PredictorDFCMGradoK: PredictorModel<T, int>
{
protected:
	int numPartesMostrar = 10000;

	AccessesDataset<T, T> datos;

	bool accederTablaInstrHash(T instruccion, shared_ptr<HistoryCacheEntry<T, T, T>>* entry) {
		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
		bool entryIsFound = this->tablaInstrHash->getEntry(instruccion, firstTableEntry.get());
		if (!entryIsFound) {
			return false;
		}
		else {
			*entry = firstTableEntry;
			return true;
		}
	}

	bool accederTablaHashDelta(T hash, shared_ptr<HistoryCacheEntry<T, T, Delta>>* entry) {
		shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());
		bool entryIsFound = this->tablaHashDelta->getEntry(hash, secondTableEntry.get());
		if (!entryIsFound) {
			return false;
		}
		else {
			*entry = secondTableEntry;
			return true;
		}
	}

	bool escribirTablaInstrHash(T instruccion, T ultimoAcceso, T delta) {
		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
		bool estabaEnTabla = accederTablaInstrHash(instruccion, &firstTableEntry);
		// T h;
		// tablaInstrHash[instruccion] = tuple<T,T>(ultimoAcceso, hash);
		this->tablaInstrHash->newAccess(instruccion, ultimoAcceso, delta);
		return estabaEnTabla;
	}


	bool escribirTablaHashDelta(T hash, Delta delta) {
		shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());
		bool estabaEnTabla = accederTablaHashDelta(hash, &secondTableEntry);
		this->tablaHashDelta->newAccess(hash, delta, 0);
		return estabaEnTabla;
	}

public:
	
	long numAciertos = 0;

	double tasaExito = 0.0;


	shared_ptr <HistoryCache<T, T, T, T>> tablaInstrHash;
	shared_ptr <HistoryCache<T, T, T, Delta>> tablaHashDelta;

	HistoryCacheType historyCacheType;
	CacheParameters firstTableCacheParams = {};
	CacheParameters secondTableCacheParams = {};


	PredictorDFCMGradoK(AccessesDataset<T,T>& datos, HistoryCacheType historyCacheType, CacheParameters firstTableCacheParams = {},
		CacheParameters secondTableCacheParams = {}) {
		this->datos = datos;
		this->historyCacheType = historyCacheType;
		this->firstTableCacheParams = firstTableCacheParams;
		this->secondTableCacheParams = secondTableCacheParams;
		inicializarPredictor();
	}

	PredictorDFCMGradoK(HistoryCacheType historyCacheType,CacheParameters firstTableCacheParams = {},
		CacheParameters secondTableCacheParams = {}) {
		this->historyCacheType = historyCacheType;
		this->firstTableCacheParams = firstTableCacheParams;
		this->secondTableCacheParams = secondTableCacheParams;
		inicializarPredictor();
	}


	~PredictorDFCMGradoK() {
		this->clean();
	}

	void clean() {
		
		this->tablaInstrHash->clean();
		this->tablaHashDelta->clean();
		
		this->datos = {};
	}

	void importarDatos(AccessesDataset<T, T>& datos, BuffersDataset<int>& datasetClases) {
		this->datos = datos;
	}

	void inicializarPredictor() {
		if(historyCacheType == HistoryCacheType::Infinite) {
			this->tablaInstrHash =
				shared_ptr<HistoryCache< T, T, T, T >>(
					new InfiniteHistoryCache< T, T, T, T >(1, 1));

			this->tablaHashDelta =
				shared_ptr<HistoryCache< T, T, T, Delta >>(
					new InfiniteHistoryCache< T, T, T, Delta >(1, 1));
		}
		else if (historyCacheType == HistoryCacheType::Real) {
			this->tablaInstrHash =
				shared_ptr<HistoryCache< T, T, T, T >>(new RealHistoryCache< T, T, T, T >(this->firstTableCacheParams.numIndexBits,
					this->firstTableCacheParams.numWays, firstTableCacheParams.numSequenceAccesses, 1));

			this->tablaHashDelta =
				shared_ptr<HistoryCache< T, T, T, Delta >>(new RealHistoryCache< T, T, T, Delta >(this->secondTableCacheParams.numIndexBits,
					this->secondTableCacheParams.numWays, 1, 1));
		}
		else {
			// this->historyCache = HistoryCache<T, I, A, LA>();
			// throw -1;
			this->tablaInstrHash = nullptr;
			this->tablaHashDelta = nullptr;
		}
	}


	void ajustarPredictor(T instruccion, T acceso) {
		// Primera tabla
		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());

		T hash;
		T accesoAnterior;
		Delta delta;
		// bool hashEnTabla = accederTablaInstrHash(instruccion, &accesoAnterior, &hash);
		bool hashEnTabla = accederTablaInstrHash(instruccion, &firstTableEntry);
		
		if (!hashEnTabla) {
			accesoAnterior = acceso;
			hash = 0;
			delta = 0;
			escribirTablaInstrHash(instruccion, accesoAnterior, hash);
			escribirTablaHashDelta(hash, delta);
		}
		else {
			accesoAnterior = firstTableEntry->getLastAccess();
			delta = acceso - accesoAnterior;
			for (auto delta_ : firstTableEntry->getHistory())
				hash = hash ^ delta_;
			escribirTablaHashDelta(hash, delta);
			escribirTablaInstrHash(instruccion, acceso, delta);
		}
		
	}

	bool predecir(T instruccion, T* acceso, bool* instrEnTabla, bool* hashEnTabla) {
		T hash;
		T ultimoAcceso;
		*instrEnTabla = false;
		*hashEnTabla = false;

		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
		shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());

		// *instrEnTabla = accederTablaInstrHash(instruccion, &ultimoAcceso, &hash);
		*instrEnTabla = accederTablaInstrHash(instruccion, &firstTableEntry);

		if (!(*instrEnTabla)) return false;
		else {
			ultimoAcceso = firstTableEntry->getLastAccess();
			for (auto delta_ : firstTableEntry->getHistory())
				hash = hash ^ delta_;
			Delta delta;
			// *hashEnTabla = accederTablaHashDelta(hash, &delta);
			*hashEnTabla = accederTablaHashDelta(hash, &secondTableEntry);
			if (!(*hashEnTabla)) 
				return false;
			else {
				delta = secondTableEntry->getLastAccess();
				*acceso =
					ultimoAcceso + delta;
			}
		}
		return true;
	}

	bool predecir(T instruccion, T* acceso) {
		bool a, b;
		return prededir(instruccion, acceso, &a, &b);
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

	/*
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
			escribirTablaHashDelta(hash, delta);
			escribirTablaInstrHash(instruccion, acceso, delta);
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
	*/


	double getMemoryCosts(double* firstTableCost, double* secondTableCost) {
		*firstTableCost = (2 + this->firstTableCacheParams.numSequenceAccesses) * sizeof(T) * this->tablaInstrHash->getNumEntries();
		*secondTableCost = 2 * sizeof(T) * this->tablaHashDelta->getNumEntries();
		return *firstTableCost + *secondTableCost;
	}
};


