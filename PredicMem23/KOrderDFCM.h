/////////////////////////////////////////////////////////////////////////////
//                                                                             //
//    Copyright (c) 2024  Pablo Sánchez Cuevas                                //
//                                                                             //
//    This file is part of PredicMem23.                                         //
//                                                                             //
//    PredicMem23 is free software: you can redistribute it and/or modify       //
//    it under the terms of the GNU General Public License as published by     //
//    the Free Software Foundation, either version 3 of the License, or        //
//    (at your option) any later version.                                      //
//                                                                             //
//    PredicMem23 is distributed in the hope that it will be useful,           //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the              //
//    GNU General Public License for more details.                             //
//                                                                             //
//    You should have received a copy of the GNU General Public License        //
//    along with PredicMem23. If not, see <http://www.gnu.org/licenses/>.       //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <iostream>
#include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"
#include "PredictorModel.h"
#include "DFCM.h"

using namespace std;

/**
 * @class KOrderDFCM
 * @brief A class for the K-order DFCM predictor.
 *
 * This class implements a predictor model using a K-order DFCM (Delta-Feedback Cache Model) with two cache tables.
 * It supports prediction, fitting, and simulation using history-based accesses.
 *
 * @tparam T The type of the data (e.g., integer or address).
 * @tparam Delta The type of the delta values (e.g., integer).
 */
template<typename T, typename Delta>
class KOrderDFCM : public PredictorModel<T, int> {
protected:
	int numPartsToPrint = 10000; ///< Number of parts to print during simulation.
	AccessesDataset<T, T> data;  ///< Dataset for accesses.

	/**
	 * @brief Access the instruction history cache table.
	 *
	 * @param instruction The instruction to look up in the table.
	 * @param entry A pointer to the entry found in the cache.
	 * @return True if the instruction is found in the table, false otherwise.
	 */
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

	/**
	 * @brief Access the hash delta history cache table.
	 *
	 * @param hash The hash value to look up in the table.
	 * @param entry A pointer to the entry found in the cache.
	 * @return True if the hash is found in the table, false otherwise.
	 */
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

	/**
	 * @brief Write an instruction and its corresponding access and delta to the instruction hash table.
	 *
	 * @param instruction The instruction to write.
	 * @param lastAccess The last access time of the instruction.
	 * @param delta The delta value to associate with the instruction.
	 * @return True if the instruction was previously in the table, false otherwise.
	 */
	bool writeInstrHashTable(T instruction, T lastAccess, T delta) {
		shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
		bool wasInTable = accessInstrHashTable(instruction, &firstTableEntry);
		this->instrHashTable->newAccess(instruction, lastAccess, delta);
		return wasInTable;
	}

	/**
	 * @brief Write a delta value to the hash delta table.
	 *
	 * @param hash The hash value to write to the table.
	 * @param delta The delta value to associate with the hash.
	 * @return True if the hash was previously in the table, false otherwise.
	 */
	bool writeHashDeltaTable(T hash, Delta delta) {
		shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
			shared_ptr< HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());
		bool wasInTable = accessHashDeltaTable(hash, &secondTableEntry);
		this->hashDeltaTable->newAccess(hash, delta, 0);
		return wasInTable;
	}

public:
	long numHits = 0; ///< Number of hits during prediction.
	double hitRate = 0.0; ///< Hit rate during simulation.

	shared_ptr<HistoryCache<T, T, T, T>> instrHashTable; ///< Instruction hash table.
	shared_ptr<HistoryCache<T, T, T, Delta>> hashDeltaTable; ///< Hash delta table.

	HistoryCacheType historyCacheType; ///< Type of history cache (Infinite or Real).
	CacheParameters firstTableCacheParams = {}; ///< Cache parameters for the first table.
	CacheParameters secondTableCacheParams = {}; ///< Cache parameters for the second table.
	bool countMemoryCapacity = false; ///< Flag to indicate if memory capacity should be counted.

	/**
	 * @brief Constructor for KOrderDFCM with a given dataset and cache type.
	 *
	 * @param data The dataset containing accesses and instructions.
	 * @param historyCacheType The type of the history cache (Infinite or Real).
	 * @param firstTableCacheParams Cache parameters for the first table.
	 * @param secondTableCacheParams Cache parameters for the second table.
	 * @param countTotalMemoryCost Flag to indicate if total memory cost should be considered.
	 */
	KOrderDFCM(AccessesDataset<T, T>& data, HistoryCacheType historyCacheType,
		CacheParameters firstTableCacheParams = {}, CacheParameters secondTableCacheParams = {},
		bool countTotalMemoryCost = true) {
		this->data = data;
		this->historyCacheType = historyCacheType;
		this->firstTableCacheParams = firstTableCacheParams;
		this->secondTableCacheParams = secondTableCacheParams;
		initializePredictor();
		this->countMemoryCapacity = !countTotalMemoryCost;
	}

	/**
	 * @brief Constructor for KOrderDFCM without a given dataset (only cache type).
	 *
	 * @param historyCacheType The type of the history cache (Infinite or Real).
	 * @param firstTableCacheParams Cache parameters for the first table.
	 * @param secondTableCacheParams Cache parameters for the second table.
	 * @param countTotalMemoryCost Flag to indicate if total memory cost should be considered.
	 */
	KOrderDFCM(HistoryCacheType historyCacheType, CacheParameters firstTableCacheParams = {},
		CacheParameters secondTableCacheParams = {}, bool countTotalMemoryCost = true) {
		this->historyCacheType = historyCacheType;
		this->firstTableCacheParams = firstTableCacheParams;
		this->secondTableCacheParams = secondTableCacheParams;
		initializePredictor();
		this->countMemoryCapacity = !countTotalMemoryCost;
	}

	/**
	 * @brief Destructor for KOrderDFCM, cleans up resources.
	 */
	~KOrderDFCM() {
		this->clean();
	}

	/**
	 * @brief Clean up the KOrderDFCM resources (hash tables and dataset).
	 */
	void clean() {
		this->instrHashTable->clean();
		this->hashDeltaTable->clean();
		this->data = {};
	}

	/**
	 * @brief Import new data into the predictor.
	 *
	 * @param data The dataset to import.
	 * @param datasetClases The class dataset for experimentation.
	 */
	void importData(AccessesDataset<T, T>& data, BuffersDataset<int>& datasetClases) {
		this->data = data;
	}

	/**
	 * @brief Initialize the predictor based on the cache type.
	 */
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

	/**
	 * @brief Fits the predictor model with the given instruction and access.
	 *
	 * This function updates the predictor's history tables (instrHashTable and hashDeltaTable) based on the provided instruction and its corresponding access.
	 * It checks if the instruction is already present in the history table and updates the history accordingly.
	 *
	 * @param instruccion The instruction to be processed.
	 * @param acceso The access associated with the instruction.
	 */
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

	/**
	 * @brief Predicts the next access for the given instruction using the predictor model.
	 *
	 * This function tries to predict the next access for a given instruction by looking up both the instruction
	 * and its corresponding delta history in the history tables.
	 * It updates the `access` and `instrIsInTable`/`hashIsInTable` flags to reflect the results of the prediction.
	 *
	 * @param instruction The instruction to predict the next access for.
	 * @param access Pointer to store the predicted access.
	 * @param instrIsInTable Output flag indicating if the instruction is present in the history table.
	 * @param hashIsInTable Output flag indicating if the hash is present in the delta table.
	 * @return True if the prediction was successful, false otherwise.
	 */
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

	/**
	 * @brief Predicts the next access for the given instruction using the predictor model, with simplified parameters.
	 *
	 * This is a simplified version of the `predict` function where the `instrIsInTable` and `hashIsInTable` flags are
	 * not required to be explicitly passed by the user.
	 *
	 * @param instruction The instruction to predict the next access for.
	 * @param access Pointer to store the predicted access.
	 * @return True if the prediction was successful, false otherwise.
	 */
	bool predict(T instruction, T* access) {
		bool a, b;
		return predict(instruction, access, &a, &b);
	}

	/**
	 * @brief Simulates the predictor model on the given dataset and returns the results and memory costs.
	 *
	 * This function simulates the operation of the predictor on the provided dataset of accesses. It tracks the number of hits,
	 * misses, and calculates the memory cost of the model based on the simulation results.
	 *
	 * @param inicializar A flag indicating whether to reinitialize the predictor before simulation.
	 * @return A shared pointer to the simulation results, including hit rate, miss rate, and memory costs.
	 */
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

	/**
	 * @brief Calculates the total memory cost of the predictor's history tables.
	 *
	 * This function calculates the total memory cost of the predictor, including both the first and second history tables,
	 * based on the number of entries and the bit size of each entry.
	 *
	 * @param firstTableCost Pointer to store the memory cost of the first table.
	 * @param secondTableCost Pointer to store the memory cost of the second table.
	 * @return The total memory cost of the predictor.
	 */
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

	/**
	 * @brief Calculates the memory cost of the predictor's history tables based on a simplified memory model.
	 *
	 * This function calculates the memory cost of the first and second history tables based on the number of entries and
	 * the bit size of each entry, assuming a simplified memory model.
	 *
	 * @param firstTableCost Pointer to store the memory cost of the first table.
	 * @param secondTableCost Pointer to store the memory cost of the second table.
	 * @return The total memory cost of the predictor.
	 */
	double getMemoryCosts(double* firstTableCost, double* secondTableCost) {
		int wordSize = sizeof(T) * 8;
		double firstTableEntryNumBits = wordSize * (1 + this->firstTableCacheParams.numSequenceAccesses);
		double secondTableEntryNumBits = wordSize;
		*firstTableCost = firstTableEntryNumBits * instrHashTable->getNumEntries() / 8.0;
		*secondTableCost = secondTableEntryNumBits * hashDeltaTable->getNumEntries() / 8.0;
		return *firstTableCost + *secondTableCost;
	}
};


