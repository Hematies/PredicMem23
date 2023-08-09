#pragma once
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>
#include "PredictorSVM.h"
#include "Global.h"

using namespace std;


template<typename T, typename A, typename LA>
class HistoryCacheEntry {

public:
	
	virtual bool isEntryValid() = 0;
	virtual void setEntry(T,LA,A) = 0;
	virtual void copy(HistoryCacheEntry<T, A, LA>*) = 0;

	virtual vector<A> getHistory() = 0;
	virtual void setHistory(vector<A>) = 0;
	virtual T getTag() = 0;
	virtual void setTag(T) = 0;
	//virtual int getWay();
	//virtual void setWay();
	virtual LA getLastAccess() = 0;
	virtual void setLastAccess(LA) = 0;
	virtual void clear() = 0;
	
};

template<typename T,typename A, typename LA>
class StandardHistoryCacheEntry : public HistoryCacheEntry<T, A, LA>{

	
protected:
	vector<A> history;
	T tag;
	LA lastAccess;
public:
	
	//friend class RealHistoryCacheEntry<T, A, LA>;

	StandardHistoryCacheEntry();
	StandardHistoryCacheEntry(int numAccesses);
	~StandardHistoryCacheEntry() {
		// history.clear();
	}

	bool isEntryValid();
	void setEntry(T newTag, LA access, A class_);
	void copy(HistoryCacheEntry<T, A, LA>*);

	vector<A> getHistory();
	void setHistory(vector<A> h);
	T getTag();
	void setTag(T t);
	LA getLastAccess();
	void setLastAccess(LA la);
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

template<typename T, typename A, typename LA>
class RealHistoryCacheEntry : public StandardHistoryCacheEntry<T, A, LA> {
protected:
	//vector<A> history;
	int way;
	//int numAccesses;
public:

	RealHistoryCacheEntry();
	RealHistoryCacheEntry(int numAccesses, int way);
	~RealHistoryCacheEntry() {
		// this->history.clear();
	}

	void copy(HistoryCacheEntry<T, A, LA>*);
	
	int getWay() {
		return this->way;
	}

	void setWay(int way) {
		this->way = way;
	}

};

template<typename T, typename I, typename A, typename LA>
class HistoryCacheSet {
protected: 
	vector<RealHistoryCacheEntry<T, A, LA>> entries;
	vector<bool> isEntryRecentlyUsed;
	int numTagBits;
	int numAccesses;
	int headWay;

	vector<int> entriesConfidence = vector<int>();
	int numConfidenceLevels = 255;
	int numConfidenceJumps = 8;

public:

	HistoryCacheSet();
	HistoryCacheSet(int numWays, int numTagBits, int numAccesses);

	~HistoryCacheSet() {
		clean();
	}

	int getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	bool newAccess(I instruction, LA access, A class_);
	void updateLRU(int newAccessWay);
	void updateLFU(int newAccessWay);
	int getLeastRecentWay();
	int getLeastFrequentWay();

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

template<typename T, typename I, typename A, typename LA>
class HistoryCache {
public:

	virtual bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res) = 0;
	virtual bool newAccess(I, LA, A) = 0;
	virtual void clean() = 0;
	virtual long getNumEntries() = 0;

	virtual double getMemoryCost() = 0;
	virtual double getTotalMemoryCost() = 0;
};

enum HistoryCacheType { Infinite = 0 , Real = 1};

template<typename T, typename I, typename A, typename LA>
class InfiniteHistoryCache : public HistoryCache<T, I, A, LA> {
friend class RealHistoryCacheEntry<T, A, LA>;
private:
	map<I, StandardHistoryCacheEntry<T, A, LA>> entries;
protected:
	int numAccesses;
	int numClasses;
public:

	InfiniteHistoryCache();
	InfiniteHistoryCache(int numAccesses, int numClasses);
	
	~InfiniteHistoryCache() {
		entries.clear();
	}
	// InfiniteHistoryCache(InfiniteHistoryCache&);

	// InfiniteHistoryCache(InfiniteHistoryCache<T, I, A, LA>&);

	bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	bool newAccess(I instruction, LA access, A class_);
	double getMemoryCost();
	double getTotalMemoryCost();


	void clean() {
		entries.clear();
	}

	long getNumEntries() {
		return entries.size();
	}

};

template<typename T, typename I, typename A, typename LA>
class RealHistoryCache : public HistoryCache<T, I, A, LA>, public InfiniteHistoryCache<T, I, A, LA> {
protected:
	// map<I, RealHistoryCache<T, A, LA>> entries;
	vector<HistoryCacheSet<T, I, A, LA>> sets;
	int numWays;
	int numIndexBits;
	// int numAccesses;
public:

	RealHistoryCache();
	RealHistoryCache(int numIndexBits, int numWays, int numAccesses, int numClasses);

	~RealHistoryCache() {
		clean();
	}

	bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	bool newAccess(I instruction, LA access, A class_);
	double getMemoryCost();
	double getTotalMemoryCost();

	void clean() {
		// sets.clear();
		// sets = vector<HistoryCacheSet<T, I, A, LA>>();
		for (auto& set : sets)
			set.clean();
	}

	int getNumWays() {
		return this->numWays;
	}

	long getNumSets() {
		return this->sets.size();;
	}

	int getNumIndexBits() {
		return numIndexBits;
	}

	long getNumEntries() {
		return sets.size() * numWays;
	}

};


template<typename D>
struct DictionaryEntry {
	D delta;
	int confidence;
};

template<typename D>
class Dictionary {
public:
	int numClasses;
	int maxConfidence;
	int numConfidenceJumps;
	vector<DictionaryEntry<D>> entries;

	Dictionary();
	Dictionary(int numClasses, int maxConfidence = 255, int numConfidenceJumps = 8);
	// Dictionary(const Dictionary&);

	~Dictionary() {
		entries.clear();
	}

	int leastReliableClass();
	int newDelta(D delta);
	int getClass(D delta);
	void showContent();

	double getMemoryCost() {
		double costPerEntry = sizeof(D); // Delta value. There are no class bits.
		return costPerEntry * this->entries.size();
	}
	double getTotalMemoryCost() {
		double extraCostPerEntry = ceil(log10(this->maxConfidence) / log10(2)); // Confidence value bits
		return (extraCostPerEntry / 8) * this->entries.size() + getMemoryCost();
	}

	// Dictionary copy();

};


template<typename T, typename I, typename A, typename LA, typename Delta>
class BuffersSimulator {
public: 
	shared_ptr<HistoryCache<T, I, A, LA>> historyCache;
	Dictionary<Delta> dictionary;
	int numHistoryAccesses;
	bool saveHistoryAndClassAfterDictMiss;
	bool saveHistoryAndClassIfNotValid;

	BuffersSimulator() {
		this->historyCache = nullptr;
		this->dictionary = Dictionary<Delta>();
		this->saveHistoryAndClassAfterDictMiss = false;
		this->saveHistoryAndClassIfNotValid = false;
		numHistoryAccesses = 0;
	}

	BuffersSimulator(HistoryCacheType cacheType, CacheParameters cacheParams, DictionaryParameters dictParams);

	BuffersSimulator(const BuffersSimulator& b);

	/*
	~BuffersSimulator() {
		clean();
	}
	*/

	void clean() {
		if(this->historyCache != nullptr)
			this->historyCache->clean();
		this->dictionary.~Dictionary();
	}

	BuffersDataset<A> simulate(AccessesDataset<I, LA>& dataset);
	
	shared_ptr<PredictResultsAndCosts> simulateWithSVM(AccessesDataset<I, LA>& dataset, 
		shared_ptr<PredictorSVM<MultiSVMClassifierOneToAll, int>>& model,
		bool initializeModel = true);
	

	bool testBuffers(I instruction, LA currentAccess, LA previousAccess);

	double getMemoryCost() {
		return this->historyCache->getMemoryCost() + this->dictionary.getMemoryCost();
	}
	double getTotalMemoryCost() {
		return this->historyCache->getTotalMemoryCost() + this->dictionary.getTotalMemoryCost();
	}

	// BuffersSimulator<T,I,A,LA> copy();
};

BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>
proposedBuffersSimulator(AccessesDataset<L64bu, L64bu>& dataset, BuffersDataset<int>& classesDataset,
	CacheParameters cacheParams, DictionaryParameters dictParams);

