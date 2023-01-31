#pragma once

struct ModelParameters {

};

struct CacheParameters {
	int numIndexBits;
	int numWays;
	int numSequenceAccesses;
};

struct DictionaryParameters {
	int numClasses;
	int numEntries;
	int maxConfidence;
	int numConfidenceJumps;
	bool saveHistoryAndClassAfterMiss;
};

struct PredictorParameters {
	CacheParameters cacheParams;
	DictionaryParameters dictParams;
};



struct PredictResults {
	double hitRate;
	double cacheMissRate;
	double dictionaryMissRate;
};

struct PredictResultsAndCosts {
	double hitRate;
	double cacheMissRate;
	double dictionaryMissRate;
	double cacheMemoryCosts;
	double dictionaryMemoryCosts;
	double modelMemoryCosts;
};
