#pragma once
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

template<typename T, typename A, typename LA>
class HistoryCacheEntry {
/*
protected:
	vector<A> history;
	T tag;
	LA lastAccess;
*/
public:
	
	//HistoryCacheEntry<T,A,LA>();
	//HistoryCacheEntry<T,A,LA>(int numAccesses);

	virtual bool isEntryValid() = 0;
	virtual void setEntry(T,LA,A) = 0;
	virtual void copy(HistoryCacheEntry<T, A, LA>*) = 0;

	virtual vector<A> getHistory() = 0;
	virtual void setHistory(vector<A>) = 0;
	virtual T getTag() = 0;
	virtual void setTag(T) = 0;
	virtual LA getLastAccess() = 0;
	virtual void setLastAccess(LA) = 0;


};

template<typename T = long,typename A = int, typename LA = long>
class ClassesHistoryCacheEntry : public virtual HistoryCacheEntry<T, A, LA>{
protected:
	vector<A> history;
	T tag;
	LA lastAccess;
public:
	
	ClassesHistoryCacheEntry();
	ClassesHistoryCacheEntry(int numClasses);

	bool isEntryValid();
	void setEntry(long newTag, long access, int class_);
	void copy(HistoryCacheEntry<T, A, LA>*);

	vector<A> getHistory();
	void setHistory(vector<A> h);
	T getTag();
	void setTag(T t);
	LA getLastAccess();
	void setLastAccess(LA la);
		
};

template<typename T, typename I, typename A, typename LA>
class HistoryCache {
private:
	int _numAccesses;
public:
	// HistoryCache<T,I,A,LA>();
	// HistoryCache<T,I,A,LA>(int numAccesses);
	virtual bool getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res) = 0;
	virtual bool newAccess(I, LA, A) = 0;
};

enum HistoryCacheType { InfiniteClasses = 0 };

template<typename T = long, typename I = long, typename A = int, typename LA = long>
class InfiniteClassesHistoryCache : public virtual HistoryCache<T, I, A, LA> {
private:
	int _numAccesses;
public:
	map<long, ClassesHistoryCacheEntry<long,int,long>> entries;

	InfiniteClassesHistoryCache();
	InfiniteClassesHistoryCache(int numAccesses);

	bool getEntry(long instruction, HistoryCacheEntry<long, int, long>* res);
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

	Dictionary();
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

template<typename T = long, typename I = long, typename A = int, typename LA = long>
class BuffersSimulator {
public: 
	HistoryCache<T, I, A, LA>* historyCache;
	Dictionary<LA> dictionary;
	int numHistoryAccesses;
	bool saveHistoryAndClassAfterDictMiss;

	BuffersSimulator(HistoryCacheType historyCacheType, int numHistoryAccesses, int numClasses,
		int maxConfidence = 255, int numConfidenceJumps = 8,
		bool saveHistoryAndClassAfterDictMiss = true);

	BuffersDataset<A> simulate(AccessesDataset<I, LA> dataset);
};

BuffersSimulator<long, long, int, long>
proposedBuffersSimulator(AccessesDataset<long, long>& dataset, BuffersDataset<int>& classesDataset,
	int numHistoryAccesses, int numClasses,
	int maxConfidence = 255, int numConfidenceJumps = 8);

