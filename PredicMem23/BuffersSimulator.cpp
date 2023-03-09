#include "BuffersSimulator.h"
#include <limits>

template<typename T, typename A, typename LA>
vector<A> StandardHistoryCacheEntry<T, A, LA>::getHistory() {
	return vector<A>(history);
}
template<typename T, typename A, typename LA>
void StandardHistoryCacheEntry<T, A, LA>::setHistory(vector<A> h) {
	history = vector<A>(h);
}
template<typename T, typename A, typename LA>
T StandardHistoryCacheEntry<T, A, LA>::getTag() {
	return tag;
}
template<typename T, typename A, typename LA>
void StandardHistoryCacheEntry<T, A, LA>::setTag(T t) {
	tag = t;
}
template<typename T, typename A, typename LA>
LA StandardHistoryCacheEntry<T, A, LA>::getLastAccess() {
	return lastAccess;
}
template<typename T, typename A, typename LA>
void StandardHistoryCacheEntry<T, A, LA>::setLastAccess(LA la) {
	lastAccess = la;
}

template<typename T, typename A, typename LA>
StandardHistoryCacheEntry<T, A, LA>::StandardHistoryCacheEntry() {
	this->history = vector<int>();
	this->lastAccess = -1L;
	this->tag = -1L;
}

template<typename T, typename A, typename LA>
StandardHistoryCacheEntry<T, A, LA>::StandardHistoryCacheEntry(int numAccesses) {
	this->history = vector<int>(numAccesses, -1);
	this->lastAccess = -1L;
	this->tag = -1L;
}

template<typename T, typename A, typename LA>
void StandardHistoryCacheEntry<T, A, LA>::copy(HistoryCacheEntry<T, A, LA>* p) {
	// p = new StandardHistoryCacheEntry();
	p->setHistory(history);
	p->setLastAccess(lastAccess);
	p->setTag(tag);
}

template<typename T, typename A, typename LA>
bool StandardHistoryCacheEntry<T, A, LA>::isEntryValid() {
	long invalidValue = -1L;
	bool historyIsValid = true;
	for (auto value : history)
		historyIsValid = historyIsValid && (value != invalidValue);
	return historyIsValid && (lastAccess != invalidValue) && (tag != invalidValue);
}

template<typename T, typename A, typename LA>
void StandardHistoryCacheEntry<T, A, LA>::setEntry(T newTag, LA access, A class_) {
	tag = newTag;
	lastAccess = access;
	for (int i = 0; i < history.size() - 1; i++) {
		history[i] = history[i + 1];
	}
	history[history.size() - 1] = class_;

}

template<typename T, typename I, typename A, typename LA >
InfiniteHistoryCache<T, I, A, LA>::InfiniteHistoryCache() {
	entries = map<I, StandardHistoryCacheEntry<T, A, LA>>();
}

template<typename T, typename I, typename A, typename LA >
InfiniteHistoryCache<T, I, A, LA>::InfiniteHistoryCache(int numAccesses, int numClasses) {
	this->numAccesses = numAccesses;
	this->numClasses = numClasses;
	entries = map<I, StandardHistoryCacheEntry<T, A, LA>>();
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
	StandardHistoryCacheEntry<T, A, LA> entry = 
		StandardHistoryCacheEntry<T, A, LA>(numAccesses);
	bool entryFound	= getEntry(instruction, &entry);
	if (!entryFound) {
		entries[instruction] = entry;
		res = false;
	}
	
	entries[instruction].setEntry(instruction, access, class_);
	return res;
}

template<typename T, typename I, typename A, typename LA >
double InfiniteHistoryCache<T, I, A, LA>::getMemoryCost() {
	double costPerEntry = sizeof(LA); // Last access value
	double numBitsClass = ceil(log10(this->numClasses) / log10(2));
	costPerEntry += (numAccesses * numBitsClass) / 8;
	return costPerEntry * this->entries.size();
}

template<typename T, typename I, typename A, typename LA >
double InfiniteHistoryCache<T, I, A, LA>::getTotalMemoryCost() {
	double extraCostPerEntry = sizeof(I); // Instruction as tag
	return extraCostPerEntry * this->entries.size() + getMemoryCost();
}

template<typename T, typename A, typename LA>
RealHistoryCacheEntry<T, A, LA>::RealHistoryCacheEntry() {
	this->history = vector<int>();
	this->lastAccess = -1L;
	this->tag = -1L;
	this->way = -1;
}

template<typename T, typename A, typename LA>
RealHistoryCacheEntry<T, A, LA>::RealHistoryCacheEntry(int numAccesses, int way) {
	this->history = vector<int>(numAccesses, -1);
	this->way = way;
	this->lastAccess = -1L;
	this->tag = -1L;
}


template<typename T, typename A, typename LA>
void RealHistoryCacheEntry<T, A, LA>::copy(HistoryCacheEntry<T, A, LA>* p) {
	*p = RealHistoryCacheEntry<T, A, LA>(this->history.size(), this->way);
	p->setHistory(this->history);
	p->setLastAccess(this->lastAccess);
	p->setTag(this->tag);
}


template<typename T, typename I, typename A, typename LA >
RealHistoryCache<T, I, A, LA>::RealHistoryCache() {
	this->numSets = 0;
	this->numWays = 0;
	sets = vector<HistoryCacheSet<T, A, LA>>();
}

template<typename T, typename I, typename A, typename LA >
RealHistoryCache<T, I, A, LA>::RealHistoryCache(int numIndexBits, int numWays, int numAccesses, int numClasses) {
	this->numAccesses = numAccesses;
	this->numIndexBits = numIndexBits;
	this->numWays = numWays;
	this->numClasses = numClasses;
	sets = vector<HistoryCacheSet<T, I, A, LA>>();

	for (int index = 0; index < pow(2, numIndexBits); index++) {
		sets.push_back(HistoryCacheSet<T, I, A, LA>(numWays, std::numeric_limits<T>::digits - numIndexBits, numAccesses));
	}
}

template<typename T, typename I, typename A, typename LA >
bool RealHistoryCache<T, I, A, LA>::getEntry(I instruction,
	HistoryCacheEntry<T, A, LA>* res) {
	int numTagBits = std::numeric_limits<T>::digits - numIndexBits;
	long index = (instruction << numTagBits) >> numTagBits;
	int way = this->sets[index].getEntry(instruction, res);

	/*
	if (way != -1) {
		cout << "(index, way) = (" << index << "," << way << ")" << endl;
	}
	*/

	return way != -1;
}

template<typename T, typename I, typename A, typename LA >
bool RealHistoryCache<T, I, A, LA>::newAccess(I instruction, LA access, A class_) {
	int numTagBits = std::numeric_limits<T>::digits - numIndexBits;
	long index = (instruction << numTagBits) >> numTagBits;
	return this->sets[index].newAccess(instruction, access, class_);
}

template<typename T, typename I, typename A, typename LA >
double RealHistoryCache<T, I, A, LA>::getMemoryCost() {
	double costPerEntry = sizeof(LA); // Last access value;
	double numBitsClass = ceil(log10(this->numClasses) / log10(2));
	costPerEntry += (this->numAccesses * numBitsClass) / 8;
	return costPerEntry * this->getNumEntries();
}

template<typename T, typename I, typename A, typename LA >
double RealHistoryCache<T, I, A, LA>::getTotalMemoryCost() {
	double extraCostPerEntry = std::numeric_limits<T>::digits - this->numIndexBits; // Tag bits
	extraCostPerEntry += this->numAccesses; // LRU bits
	extraCostPerEntry = extraCostPerEntry / 8;
	return extraCostPerEntry * this->getNumEntries() + getMemoryCost();
}

template<typename T, typename I, typename A, typename LA >
HistoryCacheSet<T, I, A, LA>::HistoryCacheSet() {
	this->numTagBits = -1;
	this->isEntryRecentlyUsed = vector<bool>();
	this->entries = vector<shared_ptr<HistoryCacheEntry<T, A, LA>>>();
	this->numAccesses = -1;
	this->headWay = -1;
}

template<typename T, typename I, typename A, typename LA >
HistoryCacheSet<T, I, A, LA>::HistoryCacheSet(int numWays, int numTagBits, int numAccesses) {
	this->numTagBits = numTagBits;
	this->isEntryRecentlyUsed = vector<bool>(numWays, false);
	this->entries = vector<RealHistoryCacheEntry<T, A, LA>>();
	this->numAccesses = numAccesses;
	this->headWay = 0;
	for (int way = 0; way < numWays; way++) {
		entries.push_back(RealHistoryCacheEntry<T,A,LA>(numAccesses, way));
	}
}

template<typename T, typename I, typename A, typename LA >
int HistoryCacheSet<T, I, A, LA>::getEntry(I instruction, HistoryCacheEntry<T, A, LA>* res) {
	int numIndexBits = std::numeric_limits<T>::digits - this->numTagBits;
	T tag = instruction >> numIndexBits;
	for (int way = 0; way < entries.size(); way++) {
		RealHistoryCacheEntry<T, A, LA> entry = entries[way];
		if (entry.getTag() == tag) {
			entry.copy(res);
			return way;
		}
	}

	return -1;
}

template<typename T, typename I, typename A, typename LA >
bool HistoryCacheSet<T, I, A, LA>::newAccess(I instruction, LA access, A class_) {
	bool res = true;
	RealHistoryCacheEntry<T, A, LA> entry =
		RealHistoryCacheEntry<T, A, LA>();

	// We get the entry corresponding the given instruction:
	int way = getEntry(instruction, &entry);
	bool entryFound = way >= 0;
	if (!entryFound) {
		// If it is not found, we will set one of the least recent entries:
		res = false;
		way = getLeastRecentWay();
	}

	// We set the entry and update the LRU system:
	int numIndexBits = std::numeric_limits<T>::digits - this->numTagBits;
	T tag = instruction >> numIndexBits;
	entries[way].setEntry(tag, access, class_);
	updateLRU(way);
	
	return res;
}

template<typename T, typename I, typename A, typename LA >
int HistoryCacheSet<T, I, A, LA>::getLeastRecentWay() {
	int tailWay = -1;
	for (int way = 0; way < this->entries.size(); way++) {
		bool isEntryObsolete = !isEntryRecentlyUsed[way];
		if (isEntryObsolete) {
			tailWay = way;
			break;
		}
	}
	
	return tailWay;
}

template<typename T, typename I, typename A, typename LA >
void HistoryCacheSet<T, I, A, LA>::updateLRU(int newAccessWay) {
	isEntryRecentlyUsed[newAccessWay] = true;
	headWay = newAccessWay;

	// If all entries were used, we reset they "recentness", except for the head:
	bool areAllEntriesRecent = true;
	for (int w = 0; w < this->entries.size(); w++) {
		areAllEntriesRecent = areAllEntriesRecent && isEntryRecentlyUsed[w];
	}
	if (areAllEntriesRecent) {
		isEntryRecentlyUsed = vector<bool>(this->entries.size(), false);
		isEntryRecentlyUsed[headWay] = true;
	}

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

	entries = vector<DictionaryEntry<D>>(numClasses, { 0L, 0 });
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
	bool classIsFound = false;
	for (int i = 0; i < entries.size(); i++) {
		auto entry = &entries[i];
		if (entry->delta == delta) {
			if (!classIsFound) {
				class_ = i;
				classIsFound = true;
			}
			entry->confidence += (this->maxConfidence + 1) / this->numConfidenceJumps;
			if (entry->confidence > this->maxConfidence)
				entry->confidence = this->maxConfidence;
		}
		else if(entry->confidence > 0)
			entry->confidence -= 1;
		
	}

	// classIsFound = class_ >= 0;

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
template<typename D >
double Dictionary<D>::getMemoryCost() {
	double costPerEntry = sizeof(D); // Delta value. There are no class bits.
	return costPerEntry * this->entries.size();
}
*/
/*
template<typename D>
double Dictionary<D>::getTotalMemoryCost() {
	double extraCostPerEntry = ceil(log10(this->maxConfidence) / log10(2)); // Confidence value bits
	return (extraCostPerEntry / 8) * this->entries.size() + getMemoryCost();
}
*/

template<typename T, typename I, typename A, typename LA, typename Delta>
BuffersSimulator <T, I, A, LA, Delta>::BuffersSimulator(HistoryCacheType historyCacheType, CacheParameters cacheParams,
	DictionaryParameters dictParams) {
	// We initialize both the cache and the dictionary:
	if (historyCacheType == HistoryCacheType::Infinite) {
		this->historyCache = 
			shared_ptr<HistoryCache< T, I, A, LA >>(
				new InfiniteHistoryCache<T, I, A, LA>(cacheParams.numSequenceAccesses, dictParams.numClasses));
	}
	else if (historyCacheType == HistoryCacheType::Real) {
		this->historyCache = 
			shared_ptr<HistoryCache< T, I, A, LA >>(new RealHistoryCache<T, I, A, LA>(cacheParams.numIndexBits, 
				cacheParams.numWays, cacheParams.numSequenceAccesses, dictParams.numClasses));
	}	
	else {
		// this->historyCache = HistoryCache<T, I, A, LA>();
		// throw -1;
		this->historyCache = nullptr;
	}
	
	this->dictionary = Dictionary<Delta>(dictParams.numClasses, dictParams.maxConfidence, dictParams.numConfidenceJumps);
	this->saveHistoryAndClassAfterDictMiss = dictParams.saveHistoryAndClassAfterMiss;
	this->numHistoryAccesses = cacheParams.numSequenceAccesses;
}

template<typename T, typename I, typename A, typename LA, typename Delta>
BuffersSimulator <T, I, A, LA, Delta>::BuffersSimulator(const BuffersSimulator <T, I, A, LA, Delta >& simulator) {
	saveHistoryAndClassAfterDictMiss = simulator.saveHistoryAndClassAfterDictMiss;
	numHistoryAccesses = simulator.numHistoryAccesses;
	dictionary = Dictionary<Delta>(simulator.dictionary);
	InfiniteHistoryCache<T, I, A, LA> cache = *((InfiniteHistoryCache<T, I, A, LA>*) & simulator.historyCache);
	historyCache = shared_ptr<HistoryCache<T, I, A, LA>>(
		new InfiniteHistoryCache<T, I, A, LA>(cache));

}

template<typename T, typename I, typename A, typename LA, typename Delta>
BuffersDataset<A> BuffersSimulator<T, I, A, LA, Delta>::simulate(AccessesDataset<I, LA> dataset) {
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
			shared_ptr< HistoryCacheEntry<T, A, LA>>(new StandardHistoryCacheEntry<T, A, LA>());
		bool historyIsFound = historyCache->getEntry(instruction, history.get());
		Delta delta;
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
			noError = this->testBuffers(instruction, access, previousAccess);

			if (!noError)
				cout << "ERROR" << endl;
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

template<typename T, typename I, typename A, typename LA, typename Delta>
bool BuffersSimulator<T, I, A, LA, Delta>::testBuffers(I instruction, LA currentAccess, LA previousAccess) {
	bool historyIsValid = true;
	auto history = 
		shared_ptr<HistoryCacheEntry<T, A, LA>>(new StandardHistoryCacheEntry<T, A, LA>());
	bool historyIsFound = historyCache->getEntry(instruction, history.get());
	LA lastAccess;
	if (!historyIsFound) {
		history.reset();
		return false;
	}

	historyIsValid = history->isEntryValid();
	lastAccess = history->getLastAccess();
	if (lastAccess != currentAccess) {
		return false;
	}
	Delta delta = lastAccess - previousAccess;
	auto savedClass = history->getHistory()[history->getHistory().size() - 1];

	auto class_ = dictionary.getClass(delta);
	bool classIsFound = class_ >= 0;
	history.reset();
	if (!classIsFound) {
		return false;
	}

	bool classesAreSame = (savedClass == class_);
	bool deltasAreSame = (delta == dictionary.entries[savedClass].delta);
	return classesAreSame && deltasAreSame;
	// return deltasAreSame;
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


BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>
proposedBuffersSimulator(AccessesDataset<L64bu, L64bu>& dataset, BuffersDataset<int>& classesDataset,
	CacheParameters cacheParams, DictionaryParameters dictParams) {
	BuffersSimulator<L64bu, L64bu, int, L64bu, L64b> res = 
		BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>(HistoryCacheType::Infinite, cacheParams, dictParams);
	classesDataset = res.simulate(dataset);
	return res;
}


