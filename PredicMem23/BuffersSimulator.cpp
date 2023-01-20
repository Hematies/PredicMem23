#include "BuffersSimulator.h"

template<typename T, typename A, typename LA>
ClassesHistoryCacheEntry<T, A, LA>::ClassesHistoryCacheEntry(int numClasses) {
	this->history = vector<int>(numClasses, -1);
	this->lastAccess = -1L;
	this->tag = -1L;
}
template<typename T, typename A, typename LA>
bool ClassesHistoryCacheEntry<T, A, LA>::isEntryValid() {
	long invalidValue = -1L;
	bool historyIsValid = true;
	for (auto value : history)
		historyIsValid = historyIsValid && (value != invalidValue);
	return historyIsValid && (lastAccess != invalidValue) && (tag != invalidValue);
}

template<typename T, typename A, typename LA>
void ClassesHistoryCacheEntry<T, A, LA>::setEntry(long newTag, long access, int class_) {
	tag = newTag;
	lastAccess = access;
	for (int i = 0; i < history.size() - 1; i++) {
		history[i] = history[i + 1];
	}
	history[history.size() - 1] = class_;

}

template<typename T, typename I, typename A, typename LA >
InfiniteClassesHistoryCache<T, I, A, LA>::InfiniteClassesHistoryCache() {
	entries = map<long, ClassesHistoryCacheEntry>();
}

template<typename T, typename I, typename A, typename LA >
InfiniteClassesHistoryCache<T, I, A, LA>::InfiniteClassesHistoryCache(int numAccesses) {
	this->_numAccesses = numAccesses;
	entries = map<long, ClassesHistoryCacheEntry>();
}

template<typename T, typename I, typename A, typename LA >
ClassesHistoryCacheEntry<long, int, long> InfiniteClassesHistoryCache<T, I, A, LA>::getEntry(long instruction) {
	if (entries.find(instruction) == entries.end())
		return nullptr;
	else
		return entries[instruction];
}

template<typename T, typename I, typename A, typename LA >
bool InfiniteClassesHistoryCache<T, I, A, LA>::newAccess(long instruction, long access, int class_) {
	bool res = true;
	ClassesHistoryCacheEntry<long, int, long> entry = getEntry(instruction);
	if (entry == nullptr) {
		entry = entries[instruction] = ClassesHistoryCacheEntry<long, int, long>(_numAccesses);
		res = false;
	}
	
	entry.setEntry(instruction, access, class_);
	return res;
}

template<typename D>
Dictionary<D>::Dictionary(int numClasses, int maxConfidence, int numConfidenceJumps) {
	this->numClasses = numClasses;
	this->maxConfidence = maxConfidence;
	this->numConfidenceJumps = numConfidenceJumps;

	entries = vector<DictionaryEntry<D>>(numClasses, { 0L, 0 });
}

template<typename D>
int Dictionary<D>::leastReliableClass() {
	int minConfidence = -1;
	int res = -1;
	for (int i = 0; i < entries.size(); i++) {
		auto entry = entries[i];
		if ((res == -1) || entry.confidence) {
			minConfidence = entry.confidence;
			res = i;
		}
	}
	return res;
}

template<typename D>
int Dictionary<D>::newDelta(D delta) {
	int leastReliableClass = this->leastReliableClass();
	int class_ = -1;
	for (int i = 0; i < entries.size(); i++) {
		auto entry = entries[i];
		if (entry.delta == delta) {
			class_ = i;
			entry.confidence += (this->maxConfidence + 1) / this->numConfidenceJumps;
			if (entry.confidence > this->maxConfidence)
				entry.confidence = this->maxConfidence;
		}
		else entry.confidence -= 1;
	}

	bool classIsFound = class_ >= 0;

	if(!classIsFound){
		class_ = leastReliableClass;
		auto entry = entries[class_];
		entry.delta = delta;
		entry.confidence = (this->maxConfidence + 1) / this->numConfidenceJumps;
	}

	return class_;
}

template<typename D>
int Dictionary<D>::getClass(D delta) {
	int class_ = -1;
	for (int i = 0; i < entries.size(); i++) {
		auto entry = entries[i];
		if (entry.delta == delta) class_ = i;
	}

	return class_;
}


template<typename CacheType, typename T, typename I, typename A, typename LA>
BuffersSimulator <CacheType, T, I, A, LA > ::BuffersSimulator(int numHistoryAccesses, int numClasses,
	int maxConfidence, int numConfidenceJumps) {
	// We initialize both the cache and the dictionary:
	this->historyCache = CacheType<T,I,A,LA>(numHistoryAccesses);
	this->dictionary = Dictionary<LA>(numClasses, maxConfidence, numConfidenceJumps);
}

template<typename CacheType, typename T, typename I, typename A, typename LA>
BuffersDataset<A> BuffersSimulator<CacheType, T, I, A, LA >::simulate(AccessesDataset<I, LA> dataset) {
	// We iterate through the given samples:
	auto accesses = dataset.accesses;
	auto instructions = dataset.accessesInstructions;

	BuffersDataset<A> res = {
		vector<vector<A>>(),
		vector<A>(),
		vector<bool>()
	};

	for (int k = 0; k < accesses.size(); k++) {
		auto access = accesses[k];
		auto instruction = instructions[k];

		vector<A> inputAccesses = vector<A>();
		A outputAccess;
		bool isValid = true;

		// First, we ask the dictionary for the class/word assigned to the access:
		auto class_ = dictionary.getClass(access);
		bool classIsFound = class_ >= 0;
		
		// Then, we ask the cache for the respective instruction history:
		bool historyIsValid = true;
		HistoryCacheEntry<T, A, LA> history = historyCache.getEntry(instruction);
		bool historyIsFound = history != nullptr;
		if (historyIsFound) historyIsValid = history.isEntryValid();
		else historyIsValid = false;

		// The history and the dictionary are updated:
		class_ = dictionary.newDelta(access);
		historyCache.newAccess(instruction, access, class_);

		if (!classIsFound || !historyIsValid) {
			// The access is labeled as miss:
			isValid = false;
			inputAccesses = vector<A>(this->numHistoryAccesses, -1);
			outputAccess = -1;
		}
		else {
			isValid = true;
			inputAccesses = vector<A>(history.history);
			outputAccess = class_;
		}

		res.inputAccesses.push_back(inputAccesses);
		res.outputAccesses.push_back(outputAccess);
		res.isValid.push_back(isValid);

	}

	return res;

}
