#pragma once
#include<stdio.h>
#include<map>
#include<string>
#include<vector>

using namespace std;

#define L64bu unsigned long long
#define L64b long long

struct ModelParameters {

};

struct CacheParameters {
	int numIndexBits;
	int numWays;
	int numSequenceAccesses;
	bool saveHistoryAndClassIfNotValid;
};

struct DictionaryParameters {
	int numClasses;
	int maxConfidence;
	int numConfidenceJumps;
	bool saveHistoryAndClassIfNotValid;
};

enum class PredictorModelType{ BufferSVM, DFCM};

struct PredictorParameters {
	PredictorModelType type;
	CacheParameters cacheParams;
	CacheParameters additionalCacheParams;
	DictionaryParameters dictParams;
};


class PredictResultsAndCosts {
public:
	virtual double getHitRate() = 0;
	virtual void setHitRate(double) = 0;
	virtual double getTotalMemoryCost() = 0;
	virtual void setTotalMemoryCost(double) = 0;
	virtual map<string, double> getResultsAndCosts() = 0;
};

class BuffersSVMPredictResultsAndCosts : PredictResultsAndCosts {
public:
	double hitRate = 0.0;
	double cacheMissRate = 0.0;
	double dictionaryMissRate = 0.0;
	double cacheMemoryCost = 0.0;
	double dictionaryMemoryCost = 0.0;
	double modelMemoryCost = 0.0;
	double totalMemoryCost = 0.0;

	BuffersSVMPredictResultsAndCosts() {}

	BuffersSVMPredictResultsAndCosts(double hitRate, double cacheMissRate, double dictionaryMissRate,
		double cacheMemoryCost, double dictionaryMemoryCost, double modelMemoryCost){
		this->hitRate = hitRate;
		this->cacheMissRate = cacheMissRate;
		this->dictionaryMissRate = dictionaryMissRate;
		this->cacheMemoryCost = cacheMemoryCost;
		this->dictionaryMemoryCost = dictionaryMemoryCost;
		this->modelMemoryCost = modelMemoryCost;
		this->totalMemoryCost = cacheMemoryCost + dictionaryMemoryCost + modelMemoryCost;
	}

	double getHitRate() { return hitRate; }
	void setHitRate(double hitRate) { this->hitRate = hitRate; }
	double getTotalMemoryCost() { return totalMemoryCost; }
	void setTotalMemoryCost(double totalMemoryCost) { this->totalMemoryCost = totalMemoryCost; }

	map<string, double> getResultsAndCosts() {
		return {
			{"hitRate", hitRate},
			{"cacheMissRate", cacheMissRate},
			{"dictionaryMissRate", dictionaryMissRate},
			{"cacheMemoryCost", cacheMemoryCost},
			{"dictionaryMemoryCost", dictionaryMemoryCost},
			{"modelMemoryCost", modelMemoryCost},
			{"totalMemoryCost", totalMemoryCost},
		};
	}
};

class DFCMPredictResultsAndCosts : PredictResultsAndCosts {
public:
	double hitRate = 0.0;
	double firstTableMissRate = 0.0;
	double secondTableMissRate = 0.0;
	double firstTableMemoryCost = 0.0;
	double secondTableMemoryCost = 0.0;
	double totalMemoryCost = 0.0;

	DFCMPredictResultsAndCosts() {}

	DFCMPredictResultsAndCosts(double hitRate, double firstTableMissRate, double secondTableMissRate,
		double firstTableMemoryCost, double secondTableMemoryCost) {
		this->hitRate = hitRate;
		this->firstTableMissRate = firstTableMissRate;
		this->secondTableMissRate = secondTableMissRate;
		this->firstTableMemoryCost = firstTableMemoryCost;
		this->secondTableMemoryCost = secondTableMemoryCost;
		this->totalMemoryCost = firstTableMemoryCost + secondTableMemoryCost;
	}

	double getHitRate() { return hitRate; }
	void setHitRate(double hitRate) { this->hitRate = hitRate; }
	double getTotalMemoryCost() { return totalMemoryCost; }
	void setTotalMemoryCost(double totalMemoryCost) { this->totalMemoryCost = totalMemoryCost; }

	map<string, double> getResultsAndCosts() {
		return {
			{"hitRate", hitRate},
			{"firstTableMissRate", firstTableMissRate},
			{"secondTableMissRate", secondTableMissRate},
			{"firstTableMemoryCost", firstTableMemoryCost},
			{"secondTableMemoryCost", secondTableMemoryCost},
			{"totalMemoryCost", totalMemoryCost},
		};
	}
};

struct TraceInfo {
	std::string name;
	std::string filename;
	unsigned long numAccesses;
};


template<typename I, typename A>
struct AccessesDataset {
	vector<A> accesses = vector<A>();
	vector<I> accessesInstructions = vector<I>();
};

template<typename A = long>
struct BuffersDataset {
	vector<vector<A>> inputAccesses = vector<vector<A>>();
	vector<A> outputAccesses = vector<A>();
	vector<bool> isCacheMiss = vector<bool>();
	vector<bool> isDictionaryMiss = vector<bool>();
	vector<bool> isValid = vector<bool>();
};
