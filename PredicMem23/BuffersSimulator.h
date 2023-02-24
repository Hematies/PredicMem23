#pragma once
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>

#define L64b unsigned long long

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
class InfiniteHistoryCacheEntry : public HistoryCacheEntry<T, A, LA>{
protected:
	vector<A> history;
	T tag;
	LA lastAccess;
public:
	
	InfiniteHistoryCacheEntry();
	InfiniteHistoryCacheEntry(int numAccesses);
	~InfiniteHistoryCacheEntry() {
		history.clear();
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
		// this->~InfiniteHistoryCacheEntry();
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
class RealHistoryCacheEntry : public InfiniteHistoryCacheEntry<T, A, LA> {
protected:
	int way;
public:

	RealHistoryCacheEntry();
	RealHistoryCacheEntry(int numAccesses, int way);
	~RealHistoryCacheEntry() {
		history.clear();
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
	vector<int> indexToNextAccess;
	int numTagBits;
	int numAccesses;
	int headWay;
public:

	HistoryCacheSet();
	HistoryCacheSet(int numWays, int numTagBits, int numAccesses);

	int getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	bool newAccess(I instruction, LA access, A class_);
	int updateLRU(int newAccessWay);
	int getLeastRecentWay();

	void clean() {
		for (RealHistoryCacheEntry<T, A, LA> entry : entries) {
			entry.reset();
			indexToNextAccess.clear();
		}
	}
};

template<typename T, typename I, typename A, typename LA>
class HistoryCache {
public:

	virtual bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res) = 0;
	virtual bool newAccess(I, LA, A) = 0;
	virtual void clean() = 0;
	virtual long getNumEntries() = 0;

};

enum HistoryCacheType { Infinite = 0 , Real = 0};

template<typename T, typename I, typename A, typename LA>
class InfiniteHistoryCache : public HistoryCache<T, I, A, LA> {
private:
	map<I, InfiniteHistoryCacheEntry<T, A, LA>> entries;
protected:
	int _numAccesses;
public:

	InfiniteHistoryCache();
	InfiniteHistoryCache(int numAccesses);
	
	~InfiniteHistoryCache() {
		entries.clear();
	}
	// InfiniteHistoryCache(InfiniteHistoryCache&);

	// InfiniteHistoryCache(InfiniteHistoryCache<T, I, A, LA>&);

	bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	bool newAccess(I instruction, LA access, A class_);
	
	void clean() {
		entries.clear();
	}

	long getNumEntries() {
		return entries.size();
	}

};

template<typename T, typename I, typename A, typename LA>
class RealHistoryCache : public HistoryCache<T, I, A, LA> {
private:
	// map<I, RealHistoryCache<T, A, LA>> entries;
	vector<HistoryCacheSet<T, A, LA>> sets;
	int numWays;
	long numSets;
	int numIndexBits;
public:

	RealHistoryCache();
	RealHistoryCache(int numIndexBits, int numWays, int numAccesses);

	~RealHistoryCache() {
		entries.clear();
	}

	bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res);
	bool newAccess(I instruction, LA access, A class_);

	void clean() {
		sets.clean();
	}

	int getNumWays() {
		return this->numWays;
	}

	long getNumSets() {
		return this->numSets;
	}

	int getNumIndexBits() {
		return numIndexBits;
	}

	long getNumEntries() {
		return numSets * numWays;
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

	// Dictionary copy();

};

template<typename I, typename A>
struct AccessesDataset {
	vector<A> accesses;
	vector<I> accessesInstructions;
};

template<typename A = long>
struct BuffersDataset {
	vector<vector<A>> inputAccesses;
	vector<A> outputAccesses;
	vector<bool> isCacheMiss;
	vector<bool> isDictionaryMiss;
	vector<bool> isValid;
};

template<typename T, typename I, typename A, typename LA>
class BuffersSimulator {
public: 
	shared_ptr<HistoryCache<T, I, A, LA>> historyCache;
	Dictionary<LA> dictionary;
	int numHistoryAccesses;
	bool saveHistoryAndClassAfterDictMiss;

	BuffersSimulator() {
		this->historyCache = nullptr;
		this->dictionary = Dictionary<LA>();
		this->saveHistoryAndClassAfterDictMiss = false;
		numHistoryAccesses = 0;
	}

	BuffersSimulator(HistoryCacheType historyCacheType, int numHistoryAccesses, int numClasses,
		int maxConfidence = 255, int numConfidenceJumps = 8,
		bool saveHistoryAndClassAfterDictMiss = true);

	BuffersSimulator(const BuffersSimulator& b);

	void clean() {
		this->historyCache->clean();
		this->dictionary.~Dictionary();
	}

	BuffersDataset<A> simulate(AccessesDataset<I, LA> dataset);

	bool testBuffers(I instruction, LA previousAccess);

	// BuffersSimulator<T,I,A,LA> copy();
};

BuffersSimulator<L64b, L64b, int, L64b>
proposedBuffersSimulator(AccessesDataset<L64b, L64b>& dataset, BuffersDataset<int>& classesDataset,
	int numHistoryAccesses, int numClasses,
	int maxConfidence = 255, int numConfidenceJumps = 8);

