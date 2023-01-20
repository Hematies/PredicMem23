#pragma once
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

template<typename T, typename A, typename LA>
class HistoryCacheEntry {
public:
	vector<A> history;
	T tag;
	LA lastAccess;
	HistoryCacheEntry<T,A,LA>();
	HistoryCacheEntry<T,A,LA>(int numAccesses);
	virtual bool isEntryValid();
	virtual void setEntry(...);

};

template<typename T = long,typename A = int, typename LA = long>
class ClassesHistoryCacheEntry : public HistoryCacheEntry<T, A, LA>{
public:
	vector<A> history;
	T tag;
	LA lastAccess;
	ClassesHistoryCacheEntry();
	ClassesHistoryCacheEntry(int numClasses);
	bool isEntryValid();
	void setEntry(long newTag, long access, int class_);

};

template<typename T, typename I, typename A, typename LA>
class HistoryCache {
private:
	int _numAccesses;
public:

	HistoryCache<T,I,A,LA>();
	HistoryCache<T,I,A,LA>(int numAccesses);
	virtual HistoryCacheEntry<T,A,LA> getEntry(I instruction);
	virtual bool newAccess(...);
};

template<typename T = long, typename I = long, typename A = int, typename LA = long>
class InfiniteClassesHistoryCache : public HistoryCache<T, I, A, LA> {
private:
	int _numAccesses;
public:
	map<long, ClassesHistoryCacheEntry<long,int,long>> entries;

	InfiniteClassesHistoryCache();
	InfiniteClassesHistoryCache(int numAccesses);

	ClassesHistoryCacheEntry<long, int, long> getEntry(long instruction);

	bool newAccess(long instruction, long access, int class_);
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
	vector<DictionaryEntry<long>> entries;

	Dictionary(int numClasses, int maxConfidence = 255, int numConfidenceJumps = 8);
	int leastReliableClass();
	int newDelta(D delta);
	int getClass(D delta);

};

template<typename I = long, typename A = long>
struct AccessesDataset {
	vector<A> accesses;
	vector<I> accessesInstructions;
};

template<typename A = long>
struct BuffersDataset {
	vector<vector<A>> inputAccesses;
	vector<A> outputAccesses;
	vector<bool> isValid;
};

template<typename CacheType, typename T = long, typename I = long, typename A = int, typename LA = long>
class BuffersSimulator {
public: 
	HistoryCache<T, I, A, LA> historyCache;
	Dictionary<LA> dictionary;
	int numHistoryAccesses;

	BuffersSimulator(int numHistoryAccesses, int numClasses,
		int maxConfidence = 255, int numConfidenceJumps = 8);

	BuffersDataset<A> simulate(AccessesDataset<I, LA> dataset);
};

BuffersSimulator<InfiniteClassesHistoryCache<>, long, long, int, long>
	proposedBuffersSimulator(AccessesDataset<long, long>& dataset, BuffersDataset<int>& classesDataset,
		int numHistoryAccesses, int numClasses,
		int maxConfidence = 255, int numConfidenceJumps = 8) {
	auto res = BuffersSimulator<InfiniteClassesHistoryCache<>, long, long, int, long>(numHistoryAccesses, numClasses,
		maxConfidence, numConfidenceJumps);
	classesDataset = res.simulate(dataset);
	return res;
}

