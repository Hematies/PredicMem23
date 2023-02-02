#include "BuffersSimulator.h"

template<typename T, typename A, typename LA>
vector<A> ClassesHistoryCacheEntry<T, A, LA>::getHistory() {
	return vector<A>(history);
}
template<typename T, typename A, typename LA>
void ClassesHistoryCacheEntry<T, A, LA>::setHistory(vector<A> h) {
	history = vector<A>(h);
}
template<typename T, typename A, typename LA>
T ClassesHistoryCacheEntry<T, A, LA>::getTag() {
	return tag;
}
template<typename T, typename A, typename LA>
void ClassesHistoryCacheEntry<T, A, LA>::setTag(T t) {
	tag = t;
}
template<typename T, typename A, typename LA>
LA ClassesHistoryCacheEntry<T, A, LA>::getLastAccess() {
	return lastAccess;
}
template<typename T, typename A, typename LA>
void ClassesHistoryCacheEntry<T, A, LA>::setLastAccess(LA la) {
	lastAccess = la;
}

template<typename T, typename A, typename LA>
ClassesHistoryCacheEntry<T, A, LA>::ClassesHistoryCacheEntry() {
	this->history = vector<int>();
	this->lastAccess = -1L;
	this->tag = -1L;
}

template<typename T, typename A, typename LA>
ClassesHistoryCacheEntry<T, A, LA>::ClassesHistoryCacheEntry(int numClasses) {
	this->history = vector<int>(numClasses, -1);
	this->lastAccess = -1L;
	this->tag = -1L;
}

template<typename T, typename A, typename LA>
void ClassesHistoryCacheEntry<T, A, LA>::copy(HistoryCacheEntry<T, A, LA>* p) {
	// p = new ClassesHistoryCacheEntry();
	p->setHistory(history);
	p->setLastAccess(lastAccess);
	p->setTag(tag);
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
void ClassesHistoryCacheEntry<T, A, LA>::setEntry(T newTag, LA access, A class_) {
	tag = newTag;
	lastAccess = access;
	for (int i = 0; i < history.size() - 1; i++) {
		history[i] = history[i + 1];
	}
	history[history.size() - 1] = class_;

}

template<typename T, typename I, typename A, typename LA >
InfiniteHistoryCache<T, I, A, LA>::InfiniteHistoryCache() {
	entries = map<I, ClassesHistoryCacheEntry<T, A, LA>>();
}

template<typename T, typename I, typename A, typename LA >
InfiniteHistoryCache<T, I, A, LA>::InfiniteHistoryCache(int numAccesses) {
	this->_numAccesses = numAccesses;
	entries = map<I, ClassesHistoryCacheEntry<T, A, LA>>();
}

/*
template<typename T, typename I, typename A, typename LA >
InfiniteHistoryCache<T, I, A, LA>::InfiniteHistoryCache(InfiniteHistoryCache<T, I, A, LA>& cache) {
	InfiniteHistoryCache<T, I, A, LA> res = InfiniteHistoryCache<T, I, A, LA>(cache);
	res.entries.clear();

}
*/


template<typename T, typename I, typename A, typename LA >
 bool InfiniteHistoryCache<T, I, A, LA>::getEntry(I instruction,
	 HistoryCacheEntry<T, A, LA>* res) {
	if (entries.find(instruction) == entries.end())
		return false;
	else {
		auto entry = entries.find(instruction)->second;
		// *res = entry;
		// *res = entry.copy();
		entry.copy(res);
		return true;
	}
		
}

template<typename T, typename I, typename A, typename LA >
bool InfiniteHistoryCache<T, I, A, LA>::newAccess(I instruction, LA access, A class_) {
	bool res = true;
	ClassesHistoryCacheEntry<T, A, LA> entry = 
		ClassesHistoryCacheEntry<T, A, LA>(_numAccesses);
	bool entryFound	= getEntry(instruction, &entry);
	if (!entryFound) {
		entries[instruction] = entry;
		res = false;
	}
	
	entries[instruction].setEntry(instruction, access, class_);
	return res;
}

template<typename D>
Dictionary<D>::Dictionary() {
	this->numClasses = 0;
	this->maxConfidence = 0;
	this->numConfidenceJumps = 0;

	entries = vector<DictionaryEntry<D>>();
}

template<typename D>
Dictionary<D>::Dictionary(int numClasses, int maxConfidence, int numConfidenceJumps) {
	this->numClasses = numClasses;
	this->maxConfidence = maxConfidence;
	this->numConfidenceJumps = numConfidenceJumps;

	entries = vector<DictionaryEntry<D>>(numClasses, { -1L, 0 });
}

/*
template<typename D>
Dictionary<D>::Dictionary(const Dictionary&)
{
}
*/

template<typename D>
int Dictionary<D>::leastReliableClass() {
	int minConfidence = -1;
	int res = -1;
	for (int i = 0; i < entries.size(); i++) {
		auto entry = entries[i];
		if ((res == -1) || (entry.confidence < minConfidence)) {
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
		auto entry = &entries[i];
		if (entry->delta == delta) {
			class_ = i;
			entry->confidence += (this->maxConfidence + 1) / this->numConfidenceJumps;
			if (entry->confidence > this->maxConfidence)
				entry->confidence = this->maxConfidence;
		}
		else if(entry->confidence > 0)
			entry->confidence -= 1;
		
	}

	bool classIsFound = class_ >= 0;

	if(!classIsFound){
		class_ = leastReliableClass;
		auto entry = &entries[class_];
		entry->delta = delta;
		entry->confidence = (this->maxConfidence + 1) / this->numConfidenceJumps;

		// this->showContent();
	}

	return class_;
}

template<typename D>
int Dictionary<D>::getClass(D delta) {
	int class_ = -1;
	for (int i = 0; i < entries.size(); i++) {
		auto entry = entries[i];
		if (entry.delta == delta) {
			class_ = i;
			break;
		}
	}

	return class_;
}

template<typename D>
void Dictionary<D>::showContent() {
	auto leastReliable = leastReliableClass();
	cout << "-----" << endl;
	for (int k = 0; k < entries.size(); k++) {
		string warning = k == leastReliable ? "!" : "";
		cout << "Entry " << k << ": " << endl;
		cout << "-> Delta: " << entries[k].delta << endl;
		cout << "-> Confidence: " << entries[k].confidence << warning << endl;
	}
	cout << "-----" << endl;
}

/*
template<typename D>
Dictionary<D> Dictionary<D>::copy() {
	Dictionary<D> res = Dictionary<D>();
	res.maxConfidence = this->maxConfidence;
	res.numClasses = this->numClasses;
	res.numConfidenceJumps = this->numConfidenceJumps;
	res.entries = vector<DictionaryEntry<D>>(this->entries);
	return res;
}
*/



template<typename T, typename I, typename A, typename LA>
BuffersSimulator <T, I, A, LA >::BuffersSimulator(HistoryCacheType historyCacheType, int numHistoryAccesses, int numClasses,
	int maxConfidence, int numConfidenceJumps, bool saveHistoryAndClassAfterDictMiss) {
	// We initialize both the cache and the dictionary:
	if (historyCacheType == HistoryCacheType::Infinite) {
		this->historyCache = 
			shared_ptr<HistoryCache< T, I, A, LA >>(new InfiniteHistoryCache<T, I, A, LA>(numHistoryAccesses));
	}
	else {
		// this->historyCache = HistoryCache<T, I, A, LA>();
		// throw -1;
		this->historyCache = nullptr;
	}
	
	this->dictionary = Dictionary<LA>(numClasses, maxConfidence, numConfidenceJumps);
	this->saveHistoryAndClassAfterDictMiss = saveHistoryAndClassAfterDictMiss;
	this->numHistoryAccesses = numHistoryAccesses;
}

template<typename T, typename I, typename A, typename LA>
BuffersSimulator <T, I, A, LA >::BuffersSimulator(const BuffersSimulator <T, I, A, LA >& simulator) {
	saveHistoryAndClassAfterDictMiss = simulator.saveHistoryAndClassAfterDictMiss;
	numHistoryAccesses = simulator.numHistoryAccesses;
	dictionary = Dictionary<LA>(simulator.dictionary);
	InfiniteHistoryCache<T, I, A, LA> cache = *((InfiniteHistoryCache<T, I, A, LA>*) & simulator.historyCache);
	historyCache = shared_ptr<HistoryCache<T, I, A, LA>>(
		new InfiniteHistoryCache<T, I, A, LA>(cache));

}

template<typename T, typename I, typename A, typename LA>
BuffersDataset<A> BuffersSimulator<T, I, A, LA >::simulate(AccessesDataset<I, LA> dataset) {
	// We iterate through the given samples:
	auto accesses = dataset.accesses;
	auto instructions = dataset.accessesInstructions;

	BuffersDataset<A> res = {
		vector<vector<A>>(),
		vector<A>(),
		vector<bool>(),
		vector<bool>(),
		vector<bool>()
	};

	for (int k = 0; k < accesses.size(); k++) {
		auto access = accesses[k];
		auto instruction = instructions[k];

		vector<A> inputAccesses = vector<A>();
		A outputAccess;
		bool isValid = true,
			isCacheMiss = false,
			isDictionaryMiss = false;

		// First, we ask the cache for the respective instruction history:
		bool historyIsValid = true;
		shared_ptr<HistoryCacheEntry<T, A, LA>> history = 
			shared_ptr< HistoryCacheEntry<T, A, LA>>(new ClassesHistoryCacheEntry<T, A, LA>());
		bool historyIsFound = historyCache->getEntry(instruction, history.get());
		LA delta;
		LA previousAccess;
		if (historyIsFound) {
			historyIsValid = history->isEntryValid();
			previousAccess = history->getLastAccess();
			delta = access - previousAccess;
		}
		else {
			historyIsValid = false;
			delta = 0;
		}

		// First, we ask the dictionary for the class/word assigned to the delta of the access:
		auto class_ = dictionary.getClass(delta);
		bool classIsFound = class_ >= 0;
		

		// The history and the dictionary are updated:
		class_ = dictionary.newDelta(delta);
		historyCache->newAccess(instruction, access, class_);
		
		bool noError = true;
		isCacheMiss = !historyIsValid;
		isDictionaryMiss = !classIsFound;
		if (!classIsFound || !historyIsValid) {
			// The access is labeled as miss:
			isValid = false;
			if (!historyIsValid || !saveHistoryAndClassAfterDictMiss) {
				inputAccesses = vector<A>(this->numHistoryAccesses, -1);
				outputAccess = -1;
			}
			else {
				// In the case that only the dictionary, failed, we
				// will indicate as resulting class the class for
				// the next iteration after updating the dictionary:
				inputAccesses = vector<A>(history->getHistory());
				outputAccess = class_;
			}
			
		}
		else {
			isValid = true;
			inputAccesses = vector<A>(history->getHistory());
			outputAccess = class_;

			// We test the buffers just in case:
			noError = this->testBuffers(instruction, previousAccess);

		}

		res.inputAccesses.push_back(inputAccesses);
		res.outputAccesses.push_back(outputAccess);
		res.isValid.push_back(isValid);
		res.isDictionaryMiss.push_back(isDictionaryMiss);
		res.isCacheMiss.push_back(isCacheMiss);

		history.reset();
	}

	return res;

}

template<typename T, typename I, typename A, typename LA>
bool BuffersSimulator<T, I, A, LA >::testBuffers(I instruction, LA previousAccess) {
	bool historyIsValid = true;
	auto history = 
		shared_ptr<HistoryCacheEntry<T, A, LA>>(new ClassesHistoryCacheEntry<T, A, LA>());
	bool historyIsFound = historyCache->getEntry(instruction, history.get());
	LA lastAccess;
	if (!historyIsFound) {
		history.reset();
		return false;
	}

	historyIsValid = history->isEntryValid();
	lastAccess = history->getLastAccess();
	LA delta = lastAccess - previousAccess;
	auto savedClass = history->getHistory()[history->getHistory().size() - 1];

	auto class_ = dictionary.getClass(delta);
	bool classIsFound = class_ >= 0;
	history.reset();
	if (!classIsFound) {
		return false;
	}
	return savedClass == class_;
}

/*
template<typename T, typename I, typename A, typename LA>
BuffersSimulator<T, I, A, LA > BuffersSimulator<T, I, A, LA >::copy() {
	BuffersSimulator<T, I, A, LA > res = BuffersSimulator<T, I, A, LA >();
	res.saveHistoryAndClassAfterDictMiss = this->saveHistoryAndClassAfterDictMiss;
	res.numHistoryAccesses = this->numHistoryAccesses;
	res.dictionary = this->dictionary.copy();
	res.historyCache = unique_ptr<HistoryCache<T, I, A, LA>>(
		new InfiniteHistoryCache<T, I, A, LA>(this->historyCache));

	return res;
}
*/


BuffersSimulator<L64b, L64b, int, L64b>
proposedBuffersSimulator(AccessesDataset<L64b, L64b>& dataset, BuffersDataset<int>& classesDataset,
	int numHistoryAccesses, int numClasses,
	int maxConfidence, int numConfidenceJumps) {
	BuffersSimulator<L64b, L64b, int, L64b> res = 
		BuffersSimulator<L64b, L64b, int, L64b>(HistoryCacheType::Infinite,
		numHistoryAccesses, numClasses,
		maxConfidence, numConfidenceJumps);
	classesDataset = res.simulate(dataset);
	return res;
}
