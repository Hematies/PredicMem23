/////////////////////////////////////////////////////////////////////////////////
/// @file BuffersSimulator.h
/// @brief Declaration of buffer simulation classes and functions.
///
/// This file contains the declarations of various classes and functions 
/// for simulating cache and buffer behaviors in a predictive memory system.
///
/// @details
/// The following classes and functions are included:
/// - StandardHistoryCacheEntry
/// - RealHistoryCacheEntry
/// - InfiniteHistoryCache
/// - RealHistoryCache
/// - HistoryCacheSet
/// - Dictionary
/// - BuffersSimulator
/// - proposedBuffersSimulator
///
/// @section LICENSE
/// Copyright (c) 2024 Pablo Sánchez Cuevas
///
/// This file is part of PredicMem23.
///
/// PredicMem23 is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// PredicMem23 is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with PredicMem23. If not, see <http://www.gnu.org/licenses/>.
///
/// @section DESCRIPTION
/// This file declares the classes and methods for simulating a predictive
/// memory system. The simulation includes various types of cache entries 
/// and caches, including standard history caches, real history caches, 
/// and infinite history caches. It also includes a dictionary class for 
/// managing deltas and a buffer simulator for simulating access patterns.
/////////////////////////////////////////////////////////////////////////////////


#pragma once
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>
#include "Global.h"


using namespace std;

/**
 * @brief Abstract class representing a history cache entry.
 *
 * @tparam T Type of the tag.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 */
template<typename T, typename A, typename LA>
class HistoryCacheEntry {

public:
	
	/**
	 * @brief Check if the entry is valid.
	 * @return True if the entry is valid, false otherwise.
	 */
	virtual bool isEntryValid() = 0;

	/**
	 * @brief Set the entry with the given tag, last access, and access.
	 * @param T The tag to set.
	 * @param LA The last access to set.
	 * @param A The access to set.
	 */
	virtual void setEntry(T,LA,A) = 0;

	/**
	 * @brief Copy the entry to another HistoryCacheEntry.
	 * @param other The other HistoryCacheEntry to copy to.
	 */
	virtual void copy(HistoryCacheEntry<T, A, LA>*) = 0;

	/**
	 * @brief Get the history of accesses.
	 * @return The history of accesses.
	 */
	virtual vector<A> getHistory() = 0;

	/**
	 * @brief Set the history of accesses.
	 * @param history The history of accesses to set.
	 */
	virtual void setHistory(vector<A>) = 0;

	/**
	 * @brief Get the tag.
	 * @return The tag.
	 */
	virtual T getTag() = 0;

	/**
	 * @brief Set the tag.
	 * @param T The tag to set.
	 */
	virtual void setTag(T) = 0;
	//virtual int getWay();
	//virtual void setWay();

	/**
	 * @brief Get the last access.
	 * @return The last access.
	 */
	virtual LA getLastAccess() = 0;

	/**
	 * @brief Set the last access.
	 * @param lastAccess The last access to set.
	 */
	virtual void setLastAccess(LA) = 0;

	/**
	 * @brief Clear the entry.
	 */
	virtual void clear() = 0;
	
};


/**
 * @brief Standard implementation of a history cache entry.
 *
 * @tparam T Type of the tag.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 */
template<typename T,typename A, typename LA>
class StandardHistoryCacheEntry : public HistoryCacheEntry<T, A, LA>{

	
protected:
	vector<A> history; ///< History vector.
	T tag; ///< Tag value.
	LA lastAccess; ///< Last access value.
public:
	
	//friend class RealHistoryCacheEntry<T, A, LA>;

	/**
	 * @brief Default constructor.
	 */
	StandardHistoryCacheEntry();

	/**
	 * @brief Constructor with a specified number of accesses.
	 * @param numAccesses The number of accesses.
	 */
	StandardHistoryCacheEntry(int numAccesses);

	/**
	 * @brief Destructor.
	 */
	~StandardHistoryCacheEntry() {
		// history.clear();
	}

	/**
	 * @brief Check if the entry is valid.
	 * @return True if the entry is valid, false otherwise.
	 */
	bool isEntryValid();
	/**
	 * @brief Set the entry with the given tag, last access, and access.
	 * @param newTag The tag to set.
	 * @param access The last access to set.
	 * @param class_ The access to set.
	 */
	void setEntry(T newTag, LA access, A class_);
	/**
	 * @brief Copy the entry to another HistoryCacheEntry.
	 * @param other The other HistoryCacheEntry to copy to.
	 */
	void copy(HistoryCacheEntry<T, A, LA>*);

	/**
	 * @brief Get the history of accesses.
	 * @return The history of accesses.
	 */
	vector<A> getHistory();
	/**
	 * @brief Set the history of accesses.
	 * @param h The history of accesses to set.
	 */
	void setHistory(vector<A> h);
	/**
	 * @brief Get the tag.
	 * @return The tag.
	 */
	T getTag();
	/**
	 * @brief Set the tag.
	 * @param t The tag to set.
	 */
	void setTag(T t);
	/**
	 * @brief Get the last access.
	 * @return The last access.
	 */
	LA getLastAccess();
	/**
	 * @brief Set the last access.
	 * @param la The last access to set.
	 */
	void setLastAccess(LA la);
	/**
	 * @brief Clear the entry.
	 */
	void clear() {
		history.clear();
		// this->~StandardHistoryCacheEntry();
	}

	/*
	int getWay() {
		return -1;
	}

	void setWay(int way) {
	}
	*/
		
};


/**
 * @brief Real history cache entry with way information.
 *
 * @tparam T Type of the tag.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 */
template<typename T, typename A, typename LA>
class RealHistoryCacheEntry : public StandardHistoryCacheEntry<T, A, LA> {
protected:
	//vector<A> history;
	int way; ///< Way number.
	//int numAccesses;
public:

	/**
	 * @brief Default constructor.
	 */
	RealHistoryCacheEntry();

	/**
	 * @brief Constructor with a specified number of accesses and way.
	 * @param numAccesses The number of accesses.
	 * @param way The way number.
	 */
	RealHistoryCacheEntry(int numAccesses, int way);

	/**
	 * @brief Destructor.
	 */
	~RealHistoryCacheEntry() {
		// this->history.clear();
	}

	/**
	 * @brief Copy the entry to another HistoryCacheEntry.
	 * @param other The other HistoryCacheEntry to copy to.
	 */
	void copy(HistoryCacheEntry<T, A, LA>*);
	
	/**
	 * @brief Get the way number.
	 * @return The way number.
	 */
	int getWay() {
		return this->way;
	}

	/**
	 * @brief Set the way number.
	 * @param way The number of ways.
	 */
	void setWay(int way) {
		this->way = way;
	}

};


/**
 * @brief Set of history cache entries.
 *
 * @tparam T Type of the tag.
 * @tparam I Type of the instruction.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 */
template<typename T, typename I, typename A, typename LA>
class HistoryCacheSet {
protected: 
	vector<RealHistoryCacheEntry<T, A, LA>> entries; ///< Cache entries.
	vector<bool> isEntryRecentlyUsed; ///< LRU status of entries.
	int numTagBits; ///< Number of tag bits.
	int numAccesses; ///< Number of accesses.
	int headWay; ///< Head way for LRU.

	vector<int> entriesConfidence; ///< Confidence levels of entries.
	int numConfidenceLevels = 255; ///< Maximum confidence level.
	int numConfidenceJumps = 8; ///< Number of confidence jumps.

public:

	/**
	 * @brief Default constructor.
	 */
	HistoryCacheSet();

	/**
	 * @brief Constructor with specified parameters.
	 * @param numWays Number of ways.
	 * @param numTagBits Number of tag bits.
	 * @param numAccesses Number of accesses.
	 */
	HistoryCacheSet(int numWays, int numTagBits, int numAccesses);

	/**
	 * @brief Destructor.
	 */
	~HistoryCacheSet() {
		clean();
	}

	/**
	 * @brief Get an entry from the cache set.
	 * @param instruction Instruction to get the entry for.
	 * @param res Pointer to store the resulting entry.
	 * @return The way number if the entry is found, -1 otherwise.
	 */
	int getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);

	/**
	 * @brief Register a new access in the cache set.
	 * @param instruction Instruction to register.
	 * @param access Access to register.
	 * @param class_ Class to register.
	 * @return True if the entry was found and updated, false otherwise.
	 */
	bool newAccess(I instruction, LA access, A class_);

	/**
	 * @brief Update the LRU status of the cache set.
	 * @param newAccessWay Way that was recently accessed.
	 */
	void updateLRU(int newAccessWay);

	/**
	 * @brief Update the LFU status of the cache set.
	 * @param newAccessWay Way that was recently accessed.
	 */
	void updateLFU(int newAccessWay);

	/**
	 * @brief Get the least recently used way in the cache set.
	 * @return The least recently used way.
	 */
	int getLeastRecentWay();

	/**
	 * @brief Get the least frequently used way in the cache set.
	 * @return The least frequently used way.
	 */
	int getLeastFrequentWay();

	/**
	 * @brief Clean the cache set.
	 */
	void clean() {
		/*
		for (RealHistoryCacheEntry<T, A, LA>& entry : entries) {
			entry.clear();
			isEntryRecentlyUsed.clear();
		}
		*/
		entries = vector<RealHistoryCacheEntry<T, A, LA>>();
		isEntryRecentlyUsed.clear();
	}
};


/**
 * @brief Abstract class representing a history cache.
 *
 * @tparam T Type of the tag.
 * @tparam I Type of the instruction.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 */
template<typename T, typename I, typename A, typename LA>
class HistoryCache {
public:

	/**
	 * @brief Get an entry from the history cache.
	 * @param instruction Instruction to get the entry for.
	 * @param res Pointer to store the resulting entry.
	 * @return True if the entry is found, false otherwise.
	 */
	virtual bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res) = 0;
	/**
	 * @brief Register a new access in the history cache.
	 * @param I Instruction to register.
	 * @param LA Access to register.
	 * @param A Class to register.
	 * @return True if the access was registered successfully, false otherwise.
	 */
	virtual bool newAccess(I, LA, A) = 0;
	/**
	 * @brief Clean the history cache.
	 */
	virtual void clean() = 0;
	/**
	 * @brief Get the number of entries in the history cache.
	 * @return The number of entries.
	 */
	virtual long getNumEntries() = 0;

	/**
	 * @brief Get the memory cost of the history cache.
	 * @return The memory cost.
	 */
	virtual double getMemoryCost() = 0;
	/**
	 * @brief Get the total memory cost of the history cache.
	 * @return The total memory cost.
	 */
	virtual double getTotalMemoryCost() = 0;
};

/**
 * @brief Enum representing the types of history caches.
 */
enum HistoryCacheType { Infinite = 0 , Real = 1};


/**
 * @brief Infinite history cache implementation.
 *
 * @tparam T Type of the tag.
 * @tparam I Type of the instruction.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 */
template<typename T, typename I, typename A, typename LA>
class InfiniteHistoryCache : public HistoryCache<T, I, A, LA> {
friend class RealHistoryCacheEntry<T, A, LA>;
private:
	map<I, StandardHistoryCacheEntry<T, A, LA>> entries; ///< Cache entries.
protected:
	int numAccesses; ///< Number of accesses.
	int numClasses; ///< Number of classes.
public:

	/**
	 * @brief Default constructor.
	 */
	InfiniteHistoryCache();
	/**
	 * @brief Constructor with specified parameters.
	 * @param numAccesses Number of accesses.
	 * @param numClasses Number of classes.
	 */
	InfiniteHistoryCache(int numAccesses, int numClasses);
	
	/**
	 * @brief Destructor.
	 */
	~InfiniteHistoryCache() {
		entries.clear();
	}
	// InfiniteHistoryCache(InfiniteHistoryCache&);

	// InfiniteHistoryCache(InfiniteHistoryCache<T, I, A, LA>&);

	/**
	 * @brief Get an entry from the infinite history cache.
	 * @param instruction Instruction to get the entry for.
	 * @param res Pointer to store the resulting entry.
	 * @return True if the entry is found, false otherwise.
	 */
	bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	/**
	 * @brief Register a new access in the infinite history cache.
	 * @param instruction Instruction to register.
	 * @param access Access to register.
	 * @param class_ Class to register.
	 * @return True if the access was registered successfully, false otherwise.
	 */
	bool newAccess(I instruction, LA access, A class_);
	/**
	 * @brief Get the memory cost of the infinite history cache.
	 * @return The memory cost.
	 */
	double getMemoryCost();
	/**
	 * @brief Get the total memory cost of the infinite history cache.
	 * @return The total memory cost.
	 */
	double getTotalMemoryCost();

	/**
	 * @brief Clean the infinite history cache.
	 */
	void clean() {
		entries.clear();
	}

	/**
	 * @brief Get the number of entries in the infinite history cache.
	 * @return The number of entries.
	 */
	long getNumEntries() {
		return entries.size();
	}

};


/**
 * @brief Real history cache implementation.
 *
 * @tparam T Type of the tag.
 * @tparam I Type of the instruction.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 */
template<typename T, typename I, typename A, typename LA>
class RealHistoryCache : public HistoryCache<T, I, A, LA>, public InfiniteHistoryCache<T, I, A, LA> {
protected:
	// map<I, RealHistoryCache<T, A, LA>> entries;
	vector<HistoryCacheSet<T, I, A, LA>> sets; ///< Cache sets.
	int numWays; ///< Number of ways.
	int numIndexBits; ///< Number of index bits.
	// int numAccesses;
public:

	/**
	 * @brief Default constructor.
	 */
	RealHistoryCache();
	/**
	 * @brief Constructor with specified parameters.
	 * @param numIndexBits Number of index bits.
	 * @param numWays Number of ways.
	 * @param numAccesses Number of accesses.
	 * @param numClasses Number of classes.
	 */
	RealHistoryCache(int numIndexBits, int numWays, int numAccesses, int numClasses);

	/**
	 * @brief Destructor.
	 */
	~RealHistoryCache() {
		clean();
	}

	/**
	 * @brief Get an entry from the real history cache.
	 * @param instruction Instruction to get the entry for.
	 * @param res Pointer to store the resulting entry.
	 * @return True if the entry is found, false otherwise.
	 */
	bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	/**
	 * @brief Register a new access in the real history cache.
	 * @param instruction Instruction to register.
	 * @param access Access to register.
	 * @param class_ Class to register.
	 * @return True if the access was registered successfully, false otherwise.
	 */
	bool newAccess(I instruction, LA access, A class_);
	/**
	 * @brief Get the memory cost of the real history cache.
	 * @return The memory cost.
	 */
	double getMemoryCost();
	/**
	 * @brief Get the total memory cost of the real history cache.
	 * @return The total memory cost.
	 */
	double getTotalMemoryCost();

	/**
	 * @brief Clean the real history cache.
	 */
	void clean() {
		// sets.clear();
		// sets = vector<HistoryCacheSet<T, I, A, LA>>();
		for (auto& set : sets)
			set.clean();
	}

	/**
	 * @brief Get the number of ways in the real history cache.
	 * @return The number of ways.
	 */
	int getNumWays() {
		return this->numWays;
	}

	/**
	 * @brief Get the number of sets in the real history cache.
	 * @return The number of sets.
	 */
	long getNumSets() {
		return this->sets.size();;
	}

	/**
	 * @brief Get the number of index bits in the real history cache.
	 * @return The number of index bits.
	 */
	int getNumIndexBits() {
		return numIndexBits;
	}

	/**
	 * @brief Get the number of entries in the real history cache.
	 * @return The number of entries.
	 */
	long getNumEntries() {
		return sets.size() * numWays;
	}

};


/**
 * @brief Struct representing a dictionary entry.
 *
 * @tparam D Type of the delta.
 */
template<typename D>
struct DictionaryEntry {
	D delta; ///< Delta value.
	int confidence; ///< Confidence level.
};

/**
 * @brief Dictionary class for managing deltas.
 *
 * @tparam D Type of the delta.
 */
template<typename D>
class Dictionary {
public:
	int numClasses; ///< Number of classes.
	int maxConfidence; ///< Maximum confidence level.
	int numConfidenceJumps; ///< Number of confidence jumps.
	vector<DictionaryEntry<D>> entries; ///< Dictionary entries.

	/**
	 * @brief Default constructor.
	 */
	Dictionary();
	/**
	 * @brief Constructor with specified parameters.
	 * @param numClasses Number of classes.
	 * @param maxConfidence Maximum confidence level.
	 * @param numConfidenceJumps Number of confidence jumps.
	 */
	Dictionary(int numClasses, int maxConfidence = 255, int numConfidenceJumps = 8);
	// Dictionary(const Dictionary&);

	/**
	 * @brief Destructor.
	 */
	~Dictionary() {
		entries.clear();
	}

	/**
	 * @brief Get the least reliable class in the dictionary.
	 * @return The index of the least reliable class.
	 */
	int leastReliableClass();
	/**
	 * @brief Register a new delta in the dictionary.
	 * @param delta The delta to register.
	 * @return The class of the delta.
	 */
	int newDelta(D delta);
	/**
	 * @brief Get the class of a given delta.
	 * @param delta The delta to get the class for.
	 * @return The class of the delta.
	 */
	int getClass(D delta);
	/**
	 * @brief Display the content of the dictionary.
	 */
	void showContent();

	/**
	 * @brief Get the memory cost of the dictionary.
	 * @return The memory cost.
	 */
	double getMemoryCost() {
		double costPerEntry = sizeof(D); // Delta value. There are no class bits.
		return costPerEntry * this->entries.size();
	}
	/**
	 * @brief Get the total memory cost of the dictionary.
	 * @return The total memory cost.
	 */
	double getTotalMemoryCost() {
		double extraCostPerEntry = ceil(log10(this->maxConfidence) / log10(2)); // Confidence value bits
		return (extraCostPerEntry / 8) * this->entries.size() + getMemoryCost();
	}

	// Dictionary copy();

};


/**
 * @brief Class for simulating buffer accesses.
 *
 * @tparam T Type of the tag.
 * @tparam I Type of the instruction.
 * @tparam A Type of the access.
 * @tparam LA Type of the last access.
 * @tparam Delta Type of the delta.
 */
template<typename T, typename I, typename A, typename LA, typename Delta>
class BuffersSimulator {
public: 
	shared_ptr<HistoryCache<T, I, A, LA>> historyCache; ///< History cache.
	Dictionary<Delta> dictionary; ///< Dictionary for managing deltas.
	int numHistoryAccesses; ///< Number of history accesses.
	bool saveHistoryAndClassAfterDictMiss; ///< Flag to save history and class after dictionary miss.
	bool saveHistoryAndClassIfNotValid; ///< Flag to save history and class if not valid.

	/**
	 * @brief Default constructor.
	 */
	BuffersSimulator() {
		this->historyCache = nullptr;
		this->dictionary = Dictionary<Delta>();
		this->saveHistoryAndClassAfterDictMiss = false;
		this->saveHistoryAndClassIfNotValid = false;
		numHistoryAccesses = 0;
	}

	/**
	 * @brief Constructor with specified parameters.
	 * @param cacheType Type of the history cache.
	 * @param cacheParams Parameters for the cache.
	 * @param dictParams Parameters for the dictionary.
	 */
	BuffersSimulator(HistoryCacheType cacheType, CacheParameters cacheParams, DictionaryParameters dictParams);

	/**
	 * @brief Copy constructor.
	 * @param b The BuffersSimulator object to copy from.
	 */
	BuffersSimulator(const BuffersSimulator& b);

	/*
	~BuffersSimulator() {
		clean();
	}
	*/

	/**
	 * @brief Clean the buffers simulator.
	 */
	void clean() {
		if(this->historyCache != nullptr)
			this->historyCache->clean();
		this->dictionary.~Dictionary();
	}

	/**
	 * @brief Simulate buffer accesses.
	 * @param dataset The dataset containing the accesses.
	 * @return The result of the simulation as BuffersDataset<A>.
	 */
	BuffersDataset<A> simulate(AccessesDataset<I, LA>& dataset);
	// void simulate(AccessesDataset<I, LA> dataset, BuffersDataset<A>&);

	/**
	 * @brief Test the buffers for consistency.
	 * @param instruction The instruction to test.
	 * @param currentAccess The current access.
	 * @param previousAccess The previous access.
	 * @return True if the buffers are consistent, false otherwise.
	 */
	bool testBuffers(I instruction, LA currentAccess, LA previousAccess);

	/**
	 * @brief Get the memory cost of the buffers simulator.
	 * @return The memory cost.
	 */
	double getMemoryCost() {
		return this->historyCache->getMemoryCost() + this->dictionary.getMemoryCost();
	}
	/**
	 * @brief Get the total memory cost of the buffers simulator.
	 * @return The total memory cost.
	 */
	double getTotalMemoryCost() {
		return this->historyCache->getTotalMemoryCost() + this->dictionary.getTotalMemoryCost();
	}

	// BuffersSimulator<T,I,A,LA> copy();
};

/**
 * @brief Function to create and configure a proposed BuffersSimulator.
 *
 * @param dataset Dataset containing the accesses.
 * @param classesDataset Dataset to store the resulting classes.
 * @param cacheParams Parameters for the cache.
 * @param dictParams Parameters for the dictionary.
 * @return BuffersSimulator<L64bu, L64bu, int, L64bu, L64b> Resulting BuffersSimulator.
 */
BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>
proposedBuffersSimulator(AccessesDataset<L64bu, L64bu>& dataset, BuffersDataset<int>& classesDataset,
	CacheParameters cacheParams, DictionaryParameters dictParams);

