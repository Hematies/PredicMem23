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
#include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"
#include "PredictorModel.h"
#include "PredictorDFCM.h"

using namespace std;


template<typename T, typename Delta>
class KOrderDFCM: PredictorModel<T, int>
{
protected:
	int numPartsToPrint = 10000;

	AccessesDataset<T, T> data;

	bool accessInstrHashTable(T instruction, shared_ptr<HistoryCacheEntry<T, T, T>>* entry) {
		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
		bool entryIsFound = this->instrHashTable->getEntry(instruction, firstTableEntry.get());
		if (!entryIsFound) {
			return false;
		}
		else {
			*entry = firstTableEntry;
			return true;
		}
	}

	bool accessHashDeltaTable(T hash, shared_ptr<HistoryCacheEntry<T, T, Delta>>* entry) {
		shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());
		bool entryIsFound = this->hashDeltaTable->getEntry(hash, secondTableEntry.get());
		if (!entryIsFound) {
			return false;
		}
		else {
			*entry = secondTableEntry;
			return true;
		}
	}

	bool writeInstrHashTable(T instruction, T lastAccess, T delta) {
		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
		bool wasInTable = accessInstrHashTable(instruction, &firstTableEntry);
		this->instrHashTable->newAccess(instruction, lastAccess, delta);
		return wasInTable;
	}


	bool writeHashDeltaTable(T hash, Delta delta) {
		shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());
		bool wasInTable = accessHashDeltaTable(hash, &secondTableEntry);
		this->hashDeltaTable->newAccess(hash, delta, 0);
		return wasInTable;
	}

public:
	
	long numHits = 0;

	double hitRate = 0.0;


	shared_ptr <HistoryCache<T, T, T, T>> instrHashTable;
	shared_ptr <HistoryCache<T, T, T, Delta>> hashDeltaTable;

	HistoryCacheType historyCacheType;
	CacheParameters firstTableCacheParams = {};
	CacheParameters secondTableCacheParams = {};
	bool countMemoryCapacity = false;


	KOrderDFCM(AccessesDataset<T,T>& data, HistoryCacheType historyCacheType, CacheParameters firstTableCacheParams = {},
		CacheParameters secondTableCacheParams = {}, bool countTotalMemoryCost = true) {
		this->data = data;
		this->historyCacheType = historyCacheType;
		this->firstTableCacheParams = firstTableCacheParams;
		this->secondTableCacheParams = secondTableCacheParams;
		initializePredictor();
		this->countMemoryCapacity = !countTotalMemoryCost;
	}

	KOrderDFCM(HistoryCacheType historyCacheType,CacheParameters firstTableCacheParams = {},
		CacheParameters secondTableCacheParams = {}, bool countTotalMemoryCost = true) {
		this->historyCacheType = historyCacheType;
		this->firstTableCacheParams = firstTableCacheParams;
		this->secondTableCacheParams = secondTableCacheParams;
		initializePredictor();
		this->countMemoryCapacity = !countTotalMemoryCost;
	}


	~KOrderDFCM() {
		this->clean();
	}

	void clean() {
		
		this->instrHashTable->clean();
		this->hashDeltaTable->clean();
		
		this->data = {};
	}

	void importData(AccessesDataset<T, T>& data, BuffersDataset<int>& datasetClases) {
		this->data = data;
	}

	void initializePredictor() {
		if(historyCacheType == HistoryCacheType::Infinite) {
			this->instrHashTable =
				shared_ptr<HistoryCache< T, T, T, T >>(
					new InfiniteHistoryCache< T, T, T, T >(this->firstTableCacheParams.numSequenceAccesses, 1));

			this->hashDeltaTable =
				shared_ptr<HistoryCache< T, T, T, Delta >>(
					new InfiniteHistoryCache< T, T, T, Delta >(1, 1));
		}
		else if (historyCacheType == HistoryCacheType::Real) {
			this->instrHashTable =
				shared_ptr<HistoryCache< T, T, T, T >>(new RealHistoryCache< T, T, T, T >(this->firstTableCacheParams.numIndexBits,
					this->firstTableCacheParams.numWays, firstTableCacheParams.numSequenceAccesses, 1));

			this->hashDeltaTable =
				shared_ptr<HistoryCache< T, T, T, Delta >>(new RealHistoryCache< T, T, T, Delta >(this->secondTableCacheParams.numIndexBits,
					this->secondTableCacheParams.numWays, 1, 1));
		}
		else {
			this->instrHashTable = nullptr;
			this->hashDeltaTable = nullptr;
		}
	}


	void fit(T instruccion, T acceso) {
		// First table:
		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());

		T hash;
		T lastAccess;
		Delta delta;
		bool isHashInTable = accessInstrHashTable(instruccion, &firstTableEntry);
		
		if (!isHashInTable) {
			lastAccess = acceso;
			hash = 0;
			delta = 0;
			writeInstrHashTable(instruccion, lastAccess, hash);
			writeHashDeltaTable(hash, delta);
		}
		else {
			lastAccess = firstTableEntry->getLastAccess();
			delta = acceso - lastAccess;
			hash = 0;
			for (auto delta_ : firstTableEntry->getHistory())
				hash = hash ^ delta_;
			writeHashDeltaTable(hash, delta);
			writeInstrHashTable(instruccion, acceso, delta);
		}
		
	}

	bool predict(T instruction, T* access, bool* instrIsInTable, bool* hashIsInTable) {
		T hash;
		T lastAccess;
		*instrIsInTable = false;
		*hashIsInTable = false;

		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
		shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());

		*instrIsInTable = accessInstrHashTable(instruction, &firstTableEntry);

		if (!(*instrIsInTable)) return false;
		else {
			lastAccess = firstTableEntry->getLastAccess();
			hash = 0;
			for (auto delta_ : firstTableEntry->getHistory())
				hash = hash ^ delta_;
			Delta delta;
			// *isHashInTable = accessHashDeltaTable(hash, &delta);
			*hashIsInTable = accessHashDeltaTable(hash, &secondTableEntry);
			if (!(*hashIsInTable)) 
				return false;
			else {
				delta = secondTableEntry->getLastAccess();
				*access =
					lastAccess + delta;
			}
		}
		return true;
	}

	bool predict(T instruction, T* access) {
		bool a, b;
		return predict(instruction, access, &a, &b);
	}

	shared_ptr<PredictResultsAndCosts> simulate(bool inicializar = true) {

		DFCMPredictResultsAndCosts resultsAndCosts = DFCMPredictResultsAndCosts();
		double numFirstTableMisses = 0.0;
		double numSecondTableMisses = 0.0;


		if (inicializar) {
			this->initializePredictor();

		}

		long numHits = 0;
		double hitRate = 0.0;

		for (int i = 0; i < data.accesses.size(); i++) {
			T input = data.accessesInstructions[i];
			T output = data.accesses[i];
			T predictedOutput;
			bool instrEnTabla, hashEnTabla;
			bool tableMiss = !predict(input, &predictedOutput, &instrEnTabla, &hashEnTabla);

			bool predictionMiss = (output != predictedOutput);
			bool miss = predictionMiss || tableMiss;

			// Si ha habido un fallo, entrenamos con la muestra de input y output:
			if (miss) {
				if (tableMiss) {
					if (!instrEnTabla)
						numFirstTableMisses++;
					if (!hashEnTabla)
						numSecondTableMisses++;
				}
			}
			else
				numHits++;

			fit(input, output);


			if (i % numPartsToPrint == 0) {
				// 
				// if (!haHabidoErrorBufferes){
				std::cout << input << " -> " << output << " vs " << predictedOutput << std::endl;
				std::cout << "Hit rate: " << (double)numHits / (i + 1) << " ; " << ((double)i) / data.accesses.size() << std::endl;
			}

		}

		hitRate = ((double)numHits) / data.accesses.size();

		resultsAndCosts.hitRate = hitRate;
		resultsAndCosts.firstTableMissRate = numFirstTableMisses / data.accesses.size();
		resultsAndCosts.secondTableMissRate = numSecondTableMisses / data.accesses.size();
		double firstTableCost, secondTableCost;
		if (this->countMemoryCapacity)
			resultsAndCosts.totalMemoryCost = getMemoryCosts(&firstTableCost, &secondTableCost);
		else
			resultsAndCosts.totalMemoryCost = getTotalMemoryCosts(&firstTableCost, &secondTableCost);
		resultsAndCosts.firstTableMemoryCost = firstTableCost;
		resultsAndCosts.secondTableMemoryCost = secondTableCost;
		return shared_ptr<PredictResultsAndCosts>((PredictResultsAndCosts*)new DFCMPredictResultsAndCosts(resultsAndCosts));
	}

	double getTotalMemoryCosts(double* firstTableCost, double* secondTableCost) {
		int wordSize = sizeof(T) * 8;
		int firstTableNumTagBits = wordSize - this->firstTableCacheParams.numIndexBits;
		double firstTableEntryNumBits = firstTableNumTagBits + wordSize * (1 + this->firstTableCacheParams.numSequenceAccesses)
			+ 1; // LRU bit
		int secondTableNumTagBits = wordSize - this->secondTableCacheParams.numIndexBits;
		double secondTableEntryNumBits = secondTableNumTagBits + wordSize
			+ 1; // LRU bit
		*firstTableCost = firstTableEntryNumBits * instrHashTable->getNumEntries() / 8.0;
		*secondTableCost = secondTableEntryNumBits * hashDeltaTable->getNumEntries() / 8.0;
		return *firstTableCost + *secondTableCost;
	}

	double getMemoryCosts(double* firstTableCost, double* secondTableCost) {
		int wordSize = sizeof(T) * 8;
		double firstTableEntryNumBits = wordSize * (1 + this->firstTableCacheParams.numSequenceAccesses);
		double secondTableEntryNumBits = wordSize;
		*firstTableCost = firstTableEntryNumBits * instrHashTable->getNumEntries() / 8.0;
		*secondTableCost = secondTableEntryNumBits * hashDeltaTable->getNumEntries() / 8.0;
		return *firstTableCost + *secondTableCost;
	}
};


