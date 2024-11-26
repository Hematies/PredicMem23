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

using namespace std;

/**
 * @brief HashOnHashDFCM Class that implements a prediction model using hash-based history tables.
 *
 * This class models a prediction system based on two-level history caches: one for instructions and another for deltas.
 * It is used to simulate and predict access patterns for given data based on previous access histories.
 *
 * @tparam T Type of instruction and access data.
 * @tparam Delta Type representing the delta between two accesses.
 */
template<typename T, typename Delta>
class HashOnHashDFCM : public PredictorModel<T, int> {
protected:
    int numPartsToPrint = 10000; /**< Number of parts to print during simulation for progress tracking. */
    AccessesDataset<T, T> data; /**< The dataset of accesses. */

    /**
     * @brief Access the instruction hash table.
     *
     * @param instruction The instruction to look up in the hash table.
     * @param entry Pointer to the entry to be filled if found.
     * @return True if the entry is found, false otherwise.
     */
    bool accessInstrHashTable(T instruction, shared_ptr<HistoryCacheEntry<T, T, T>>* entry) {
        shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
            shared_ptr<HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
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
     * @brief Access the hash delta table.
     *
     * @param hash The hash to look up in the hash delta table.
     * @param entry Pointer to the entry to be filled if found.
     * @return True if the entry is found, false otherwise.
     */
    bool accessHashDeltaTable(T hash, shared_ptr<HistoryCacheEntry<T, T, Delta>>* entry) {
        shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
            shared_ptr<HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());
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
     * @brief Write to the instruction hash table.
     *
     * @param instruction The instruction to write to the hash table.
     * @param lastAccess The last access time.
     * @param hash The hash value to be written.
     * @return True if the instruction was already in the table, false otherwise.
     */
    bool writeInstrHashTable(T instruction, T lastAccess, T hash) {
        shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
            shared_ptr<HistoryCacheEntry<T, T, T>>(new StandardHistoryCacheEntry<T, T, T>());
        bool wasInTable = accessInstrHashTable(instruction, &firstTableEntry);
        this->instrHashTable->newAccess(instruction, lastAccess, hash);
        return wasInTable;
    }

    /**
     * @brief Write to the hash delta table.
     *
     * @param hash The hash to write to the delta table.
     * @param delta The delta value to be written.
     * @return True if the hash was already in the table, false otherwise.
     */
    bool writeHashDeltaTable(T hash, Delta delta) {
        shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
            shared_ptr<HistoryCacheEntry<T, T, Delta>>(new StandardHistoryCacheEntry<T, T, Delta>());
        bool wasInTable = accessHashDeltaTable(hash, &secondTableEntry);
        this->hashDeltaTable->newAccess(hash, delta, 0);
        return wasInTable;
    }

public:
    long numHits = 0; /**< Number of hits during simulation. */
    double hitRate = 0.0; /**< The hit rate during simulation. */

    shared_ptr<HistoryCache<T, T, T, T>> instrHashTable; /**< The instruction hash table. */
    shared_ptr<HistoryCache<T, T, T, Delta>> hashDeltaTable; /**< The hash delta table. */

    HistoryCacheType historyCacheType; /**< Type of history cache (Infinite or Real). */
    CacheParameters firstTableCacheParams = {}; /**< Cache parameters for the first table. */
    CacheParameters secondTableCacheParams = {}; /**< Cache parameters for the second table. */

    bool countMemoryCapacity = false; /**< Flag to determine if memory costs are calculated. */

    /**
     * @brief Constructor for initializing the model with data and cache parameters.
     *
     * @param data The access dataset.
     * @param historyCacheType The type of the history cache (Infinite or Real).
     * @param firstTableCacheParams Parameters for the first table cache.
     * @param secondTableCacheParams Parameters for the second table cache.
     * @param countTotalMemoryCost Flag to count memory costs or not.
     */
    HashOnHashDFCM(AccessesDataset<T, T>& data, HistoryCacheType historyCacheType,
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
     * @brief Alternative constructor that doesn't require data initially.
     *
     * @param historyCacheType The type of the history cache (Infinite or Real).
     * @param firstTableCacheParams Parameters for the first table cache.
     * @param secondTableCacheParams Parameters for the second table cache.
     * @param countTotalMemoryCost Flag to count memory costs or not.
     */
    HashOnHashDFCM(HistoryCacheType historyCacheType, CacheParameters firstTableCacheParams = {},
        CacheParameters secondTableCacheParams = {}, bool countTotalMemoryCost = true) {
        this->historyCacheType = historyCacheType;
        this->firstTableCacheParams = firstTableCacheParams;
        this->secondTableCacheParams = secondTableCacheParams;
        initializePredictor();
        this->countMemoryCapacity = !countTotalMemoryCost;
    }

    /**
     * @brief Destructor for cleaning up resources.
     */
    ~HashOnHashDFCM() {
        clean();
    }

    /**
     * @brief Clean up resources and reset state.
     *
     * This method clears the history tables and resets the dataset.
     */
    void clean() {
        this->instrHashTable->clean();
        this->hashDeltaTable->clean();
        this->data = {};
    }

    /**
     * @brief Import a new access dataset.
     *
     * @param data The access dataset to import.
     * @param datasetClases The dataset of classes (not used in this function).
     */
    void importData(AccessesDataset<T, T>& data, BuffersDataset<int>& datasetClases) {
        this->data = data;
    }

    /**
 * @brief Initialize the predictor based on the history cache type.
 *
 * Sets up the instruction hash table and hash delta table according to the
 * specified cache type (Infinite or Real) and parameters.
 */
    void initializePredictor() {
        if (historyCacheType == HistoryCacheType::Infinite) {
            this->instrHashTable = shared_ptr<HistoryCache<T, T, T, T>>(
                new InfiniteHistoryCache<T, T, T, T>(1, 1));

            this->hashDeltaTable = shared_ptr<HistoryCache<T, T, T, Delta>>(
                new InfiniteHistoryCache<T, T, T, Delta>(1, 1));
        }
        else if (historyCacheType == HistoryCacheType::Real) {
            this->instrHashTable = shared_ptr<HistoryCache<T, T, T, T>>(
                new RealHistoryCache<T, T, T, T>(
                    this->firstTableCacheParams.numIndexBits,
                    this->firstTableCacheParams.numWays, 1, 1));

            this->hashDeltaTable = shared_ptr<HistoryCache<T, T, T, Delta>>(
                new RealHistoryCache<T, T, T, Delta>(
                    this->secondTableCacheParams.numIndexBits,
                    this->secondTableCacheParams.numWays, 1, 1));
        }
        else {
            this->instrHashTable = nullptr;
            this->hashDeltaTable = nullptr;
        }
    }

    /**
     * @brief Train the predictor with an instruction and its access.
     *
     * Updates the instruction hash table and hash delta table using the given
     * instruction and its corresponding access.
     *
     * @param instruction The instruction to train on.
     * @param access The access value associated with the instruction.
     */
    void fit(T instruction, T access) {
        shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
            shared_ptr<HistoryCacheEntry<T, T, T>>(
                new StandardHistoryCacheEntry<T, T, T>());

        T hash;
        T lastAccess;
        Delta delta;
        bool hashIsInTable = accessInstrHashTable(instruction, &firstTableEntry);
        if (!hashIsInTable) {
            hash = 0;
            delta = 0;
            this->writeInstrHashTable(instruction, access, hash);
        }
        else {
            lastAccess = firstTableEntry->getLastAccess();
            hash = firstTableEntry->getHistory()[0];
            delta = access - lastAccess;
            this->writeHashDeltaTable(hash, delta);
            hash = hash ^ static_cast<T>(delta);
            this->writeInstrHashTable(instruction, access, hash);
        }
    }

    /**
     * @brief Predict the access for a given instruction.
     *
     * Looks up the instruction hash table and hash delta table to predict the
     * access associated with the instruction.
     *
     * @param instruction The instruction to predict for.
     * @param access Pointer to store the predicted access value.
     * @param instrIsInTable Pointer to indicate if the instruction is in the hash table.
     * @param hashIsInTable Pointer to indicate if the hash is in the hash delta table.
     * @return True if the prediction is successful, false otherwise.
     */
    bool predict(T instruction, T* access, bool* instrIsInTable, bool* hashIsInTable) {
        T hash;
        T lastAccess;
        *instrIsInTable = false;
        *hashIsInTable = false;

        shared_ptr<HistoryCacheEntry<T, T, T>> firstTableEntry =
            shared_ptr<HistoryCacheEntry<T, T, T>>(
                new StandardHistoryCacheEntry<T, T, T>());
        shared_ptr<HistoryCacheEntry<T, T, Delta>> secondTableEntry =
            shared_ptr<HistoryCacheEntry<T, T, Delta>>(
                new StandardHistoryCacheEntry<T, T, Delta>());

        *instrIsInTable = accessInstrHashTable(instruction, &firstTableEntry);

        if (!(*instrIsInTable)) {
            return false;
        }
        else {
            lastAccess = firstTableEntry->getLastAccess();
            hash = firstTableEntry->getHistory()[0];
            *hashIsInTable = accessHashDeltaTable(hash, &secondTableEntry);
            if (!(*hashIsInTable)) {
                return false;
            }
            else {
                Delta delta = secondTableEntry->getLastAccess();
                *access = lastAccess + delta;
            }
        }
        return true;
    }

    /**
     * @brief Simplified prediction method.
     *
     * Predicts the access for the given instruction without detailed status flags.
     *
     * @param instruction The instruction to predict for.
     * @param access Pointer to store the predicted access value.
     * @return True if the prediction is successful, false otherwise.
     */
    bool predict(T instruction, T* access) {
        bool dummyInstr, dummyHash;
        return predict(instruction, access, &dummyInstr, &dummyHash);
    }

    /**
     * @brief Simulate the predictor on the dataset.
     *
     * Runs the prediction model over the dataset, tracks misses, and calculates
     * memory costs and hit rates.
     *
     * @param initialize Whether to initialize the predictor before simulation.
     * @return Shared pointer to a structure containing prediction results and costs.
     */
    shared_ptr<PredictResultsAndCosts> simulate(bool initialize = true) {
        DFCMPredictResultsAndCosts resultsAndCosts;
        double numFirstTableMisses = 0.0;
        double numSecondTableMisses = 0.0;

        if (initialize) {
            this->initializePredictor();
        }

        numHits = 0;
        hitRate = 0.0;

        for (int i = 0; i < data.accesses.size(); i++) {
            T input = data.accessesInstructions[i];
            T output = data.accesses[i];
            T predictedOutput;
            bool instrIsInTable, hashIsInTable;
            bool tableMiss = !predict(input, &predictedOutput, &instrIsInTable, &hashIsInTable);

            bool predictionMiss = (output != predictedOutput);
            bool miss = predictionMiss || tableMiss;

            if (miss) {
                if (tableMiss) {
                    if (!instrIsInTable) numFirstTableMisses++;
                    if (!hashIsInTable) numSecondTableMisses++;
                }
            }
            else {
                numHits++;
            }

            fit(input, output);

            if (i % numPartsToPrint == 0) {
                std::cout << input << " -> " << output << " vs " << predictedOutput << std::endl;
                std::cout << "Hit rate: " << static_cast<double>(numHits) / (i + 1)
                    << " ; " << static_cast<double>(i) / data.accesses.size() << std::endl;
            }
        }

        hitRate = static_cast<double>(numHits) / data.accesses.size();

        resultsAndCosts.hitRate = hitRate;
        resultsAndCosts.firstTableMissRate = numFirstTableMisses / data.accesses.size();
        resultsAndCosts.secondTableMissRate = numSecondTableMisses / data.accesses.size();
        double firstTableCost, secondTableCost;
        if (this->countMemoryCapacity) {
            resultsAndCosts.totalMemoryCost = getMemoryCosts(&firstTableCost, &secondTableCost);
        }
        else {
            resultsAndCosts.totalMemoryCost = getTotalMemoryCosts(&firstTableCost, &secondTableCost);
        }
        resultsAndCosts.firstTableMemoryCost = firstTableCost;
        resultsAndCosts.secondTableMemoryCost = secondTableCost;
        return shared_ptr<PredictResultsAndCosts>((PredictResultsAndCosts*)
            new DFCMPredictResultsAndCosts(resultsAndCosts));
    }

    /**
     * @brief Calculate total memory costs including tags and LRU bits.
     *
     * @param firstTableCost Pointer to store the cost of the first table.
     * @param secondTableCost Pointer to store the cost of the second table.
     * @return Total memory cost in bytes.
     */
    double getTotalMemoryCosts(double* firstTableCost, double* secondTableCost) {
        int wordSize = sizeof(T) * 8;
        int firstTableNumTagBits = wordSize - this->firstTableCacheParams.numIndexBits;
        double firstTableEntryNumBits = firstTableNumTagBits + wordSize * 2 + 1; // LRU bit
        int secondTableNumTagBits = wordSize - this->secondTableCacheParams.numIndexBits;
        double secondTableEntryNumBits = secondTableNumTagBits + wordSize + 1; // LRU bit
        *firstTableCost = firstTableEntryNumBits * instrHashTable->getNumEntries() / 8.0;
        *secondTableCost = secondTableEntryNumBits * hashDeltaTable->getNumEntries() / 8.0;
        return *firstTableCost + *secondTableCost;
    }

    /**
     * @brief Calculate memory costs excluding tags and LRU bits.
     *
     * @param firstTableCost Pointer to store the cost of the first table.
     * @param secondTableCost Pointer to store the cost of the second table.
     * @return Total memory cost in bytes.
     */
	double getMemoryCosts(double* firstTableCost, double* secondTableCost) {
		int wordSize = sizeof(T) * 8;
		double firstTableEntryNumBits =  wordSize * 2;
		double secondTableEntryNumBits =  wordSize; 
		*firstTableCost = firstTableEntryNumBits * instrHashTable->getNumEntries() / 8.0;
		*secondTableCost = secondTableEntryNumBits * hashDeltaTable->getNumEntries() / 8.0;
		return *firstTableCost + *secondTableCost;
	}
};


