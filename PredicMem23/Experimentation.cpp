
#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <time.h>
#include "tinyxml.h"
#include <algorithm> 
#include "Experimentation.h"

string nowDateTime() {
	// auto now = std::chrono::system_clock::now();
	auto t = std::time(nullptr);
	tm tm;
	localtime_s(&tm, &t);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");
	string now = oss.str();
	return now;
}

TracePredictExperimentation::TracePredictExperimentation(vector<Experiment*> experiments, string outputFilename) {
	// this->experiments = vector<Experiment*>(experiments);
	this->experiments = experiments;
	this->outputFilename = outputFilename;
	this->traceReader = TraceReader<L64bu, L64bu>();
	
}

TracePredictExperimentation::TracePredictExperimentation(string outputFilename,bool countTotalMemory) {
	this->experiments = vector<Experiment*>();
	this->outputFilename = outputFilename;
	this->traceReader = TraceReader<L64bu, L64bu>();
	this->countTotalMemory = countTotalMemory;
}

void TracePredictExperimentation::performExperiments() {
	for (auto& experiment : this->experiments) {
		cout << "\n=========";
		cout << "\nEXPERIMENT: " << experiment->getString() << "\n";
		experiment->performExperiment();
		experiment->clean();
	}
}

vector<Experiment*> TracePredictExperimentation::getExperiments() {
	// return vector<Experiment*>(experiments);
	return experiments;
}

void TracePredictExperimentation::setExperiments(vector<Experiment*> experiments) {
	// this->experiments = vector<Experiment*>(experiments);
	this->experiments = experiments;
}

map<string, vector<Experiment*>> TracePredictExperimentation::getExperimentsByTrace() {
	map<string, vector<Experiment*>> res = map<string, vector<Experiment*>>();
	for (auto& experiment : this->experiments) {
		string traceName = experiment->getName();
		
		if (res.find(traceName) == res.end())
			res[traceName] = vector<Experiment*>();

		res[traceName].push_back(experiment);
	}
	for (auto iter = res.begin(); iter != res.end(); ++iter) {
		string traceName = iter->first;
		std::sort(res[traceName].begin(), res[traceName].end(),
			[](Experiment* experiment1, Experiment* experiment2) -> bool
			{
				return experiment1->getStartLine() < experiment2->getStartLine();
			});
	}
	return res;
}

void TracePredictExperimentation::exportResults(string filename) {
	
	TiXmlDocument doc;
	TiXmlDeclaration decl("1.0", "", "");
	doc.InsertEndChild(decl);

	// First, we output the used predictorParams???

	
	// Then we output the info related to traces results:
	TiXmlElement* traces = new TiXmlElement("Traces");

	auto experimentsByTrace = getExperimentsByTrace();
	for (auto iter = experimentsByTrace.begin(); iter != experimentsByTrace.end(); ++iter) {
		string traceName = iter->first;
		TiXmlElement* trace = new TiXmlElement(traceName.c_str());
		map<string, double> totalResults = {};

		int numExperiments = 0;
		for (auto experiment : experimentsByTrace[traceName]) {
			if (!experiment->isNull()) {
				numExperiments++;
			}
		}
		for (auto experiment : experimentsByTrace[traceName]) {
			if (!experiment->isNull()) {

				TiXmlElement* experiment_ = new TiXmlElement(experiment->getString().c_str());
				auto results = experiment->getResultsAndCosts();
				auto params = experiment->getPredictorParams();

				// First node: related to results and costs:
				TiXmlElement* results_ = new TiXmlElement("resultsAndCosts");
				/*
				results_->SetDoubleAttribute("hitRate", results["hitRate"]);
				results_->SetDoubleAttribute("cacheMissRate", results["cacheMissRate"]);
				results_->SetDoubleAttribute("dictionaryMissRate", results["dictionaryMissRate"]);
				*/
				for (auto it = results.begin(); it != results.end(); it++) {
					results_->SetDoubleAttribute(it->first.c_str(), it->second);
				}
				experiment_->LinkEndChild(results_);

				// Second node: related to input, cache params:
				auto cacheParams = params.cacheParams;
				TiXmlElement* cacheParams_ = new TiXmlElement("cacheParams");
				cacheParams_->SetAttribute("numIndexBits", cacheParams.numIndexBits);
				cacheParams_->SetAttribute("numWays", cacheParams.numWays);
				cacheParams_->SetAttribute("numSequenceAccesses", cacheParams.numSequenceAccesses);
				cacheParams_->SetAttribute("saveHistoryAndClassIfNotValid", cacheParams.saveHistoryAndClassIfNotValid);
				experiment_->LinkEndChild(cacheParams_);

				// Third node: related to input, dictionary params:
				auto dictParams = params.dictParams;
				TiXmlElement* dictParams_ = new TiXmlElement("dictParams");
				dictParams_->SetAttribute("numClasses", dictParams.numClasses);
				dictParams_->SetAttribute("maxConfidence", dictParams.maxConfidence);
				dictParams_->SetAttribute("numConfidenceJumps", dictParams.numConfidenceJumps);
				dictParams_->SetAttribute("saveHistoryAndClassIfNotValid", dictParams.saveHistoryAndClassIfNotValid);
				experiment_->LinkEndChild(dictParams_);
			
				trace->LinkEndChild(experiment_);
				for (auto it = results.begin(); it != results.end(); it++) {
					totalResults[it->first] += it->second / numExperiments;
				}
			}

		}

		for (auto it = totalResults.begin(); it != totalResults.end(); it++) {
			trace->SetDoubleAttribute(it->first.c_str(), it->second);
		}
		traces->LinkEndChild(trace);
	}
	doc.LinkEndChild(traces);

	doc.SaveFile(filename.c_str());
}

void TracePredictExperimentation::buildExperiments(vector<TraceInfo> tracesInfo,
	PredictorParameters params, long numAccessesPerExperiment = 10000000) {


	auto pointer = this;
	for (int i = 0; i < tracesInfo.size(); i++) {
		auto name = tracesInfo[i].name;
		auto filename = tracesInfo[i].filename;

		// auto experiment = TracePredictExperiment(this, filename, name, )
		TraceReader<L64bu, L64bu> reader(filename);
		unsigned long numLines = tracesInfo[i].numAccesses;
		unsigned long k = 0;
		unsigned long k1 = numAccessesPerExperiment;
		while(true) {
			k1 = k1 > numLines ? numLines : k1;
			this->experiments.push_back(
				new TracePredictExperiment(pointer, filename, name, k, k1, params, this->countTotalMemory));
			// this->experiments.push_back(make_unique<TracePredictExperiment>(experiment));

			k += numAccessesPerExperiment;
			k1 += numAccessesPerExperiment;
			if (k >= numLines) {
				printf("");
				break;

			}
		}

	}
}

TracePredictExperiment::TracePredictExperiment(string traceFilename, string traceName, long startLine, long endLine, 
	struct PredictorParameters params, bool countTotalMemory) {
	this->traceFilename = traceFilename;
	this->traceName = traceName;
	this->startLine = startLine;
	this->endLine = endLine;
	this->countTotalMemory = countTotalMemory;

	this->predictorParams = params;
	auto cacheParams = params.cacheParams;
	auto dictParams = params.dictParams;
	
	HistoryCacheType cacheType = (cacheParams.numIndexBits > 0)? HistoryCacheType::Real : HistoryCacheType::Infinite;

	if (params.type == PredictorModelType::BufferSVM) {
		this->buffersSimulator = BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>(cacheType, cacheParams, dictParams);
		this->model = shared_ptr<PredictorModel<L64bu, int>>((PredictorModel<L64bu, int>*)
			new PredictorSVM<MultiSVMClassifierOneToAll, int>(cacheParams.numSequenceAccesses, dictParams.numClasses, 
				cacheParams.saveHistoryAndClassIfNotValid));
	}
	else {
		this->model = 
			shared_ptr<PredictorModel<L64bu, int>>((PredictorModel<L64bu, int>*) 
				new PredictorDFCMHashOnHash<L64bu, L64b>(cacheType, cacheParams, params.additionalCacheParams));
	}
	
	this->startDateTime = nowDateTime();
}

TracePredictExperiment::TracePredictExperiment(TracePredictExperimentation* framework, string traceFilename, string traceName, long startLine, long endLine,
	struct PredictorParameters params, bool countTotalMemory) {
	this->framework = framework;
	this->traceFilename = traceFilename;
	this->traceName = traceName;
	this->startLine = startLine;
	this->endLine = endLine;
	this->countTotalMemory = countTotalMemory;

	this->predictorParams = params;
	auto cacheParams = params.cacheParams;
	auto dictParams = params.dictParams;

	HistoryCacheType cacheType = (cacheParams.numIndexBits >= 0) ? HistoryCacheType::Real : HistoryCacheType::Infinite;

	if (params.type == PredictorModelType::BufferSVM) {
		this->buffersSimulator = BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>(cacheType, cacheParams, dictParams);
		this->model = shared_ptr<PredictorModel<L64bu, int>>((PredictorModel<L64bu, int>*)
			new PredictorSVM<MultiSVMClassifierOneToAll, int>(cacheParams.numSequenceAccesses, dictParams.numClasses, 
				cacheParams.saveHistoryAndClassIfNotValid));
	}
	else {
		if(params.cacheParams.numSequenceAccesses > 0)
			this->model = shared_ptr<PredictorModel<L64bu, int>>(
				(PredictorModel<L64bu, int>*) new PredictorDFCMGradoK<L64bu, L64b>(cacheType, cacheParams, params.additionalCacheParams));
		else
			this->model = shared_ptr<PredictorModel<L64bu, int>>((PredictorModel<L64bu, int>*) new PredictorDFCMHashOnHash<L64bu, L64b>(cacheType, cacheParams, params.additionalCacheParams));
	}
	this->startDateTime = nowDateTime();
}

long TracePredictExperiment::getStartLine() {
	return startLine;
}

void TracePredictExperiment::setStartLine(long startLine) {
	this->startLine = startLine;
}

long TracePredictExperiment::getEndLine() {
	return endLine;
}

void TracePredictExperiment::setEndLine(long endLine) {
	this->endLine = endLine;
}

string TracePredictExperiment::getTracefile() {
	return traceFilename;
}

void TracePredictExperiment::setTraceFile(string filename) {
	this->traceFilename = filename;
}

void TracePredictExperiment::setName(string name) {
	this->traceName = name;
}

string TracePredictExperiment::getString() {

	ostringstream res; 
	res << this->startDateTime << "::" << traceName << "_" << startLine << "_" << endLine;

	return res.str();
}

void TracePredictExperiment::setTraceName(string name) {
	this->traceName = name;
}

map<string, double> TracePredictExperiment::getResultsAndCosts() {
	return resultsAndCosts->getResultsAndCosts();
}

void TracePredictExperiment::setPredictorModel(BuffersSimulator<L64bu, L64bu, int, L64bu, L64b> bufferSimulator,
	PredictorSVM<MultiSVMClassifierOneToAll, int> model) {
	this->buffersSimulator = BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>(bufferSimulator);
	this->model = shared_ptr<PredictorModel<L64bu,int>>((PredictorModel<L64bu, int>*) & model);
}

void TracePredictExperiment::setPredictorModel(PredictorDFCMHashOnHash<L64bu, L64b> model) {
	this->model = shared_ptr<PredictorModel<L64bu, int>>((PredictorModel<L64bu, int>*) & model);
}

bool TracePredictExperiment::isNull() {
	return this->isNull_;
}

void TracePredictExperiment::performExperiment() {
	this->startDateTime = nowDateTime();

	// First, we check that we don't have to instantiate a new TraceReader:
	TraceReader<L64bu, L64bu>* traceReader = &this->framework->traceReader;
	bool isSameFile = traceReader->filename == this->traceFilename;
	bool isFileOpen = traceReader->file.is_open();

	if (!isSameFile || !isFileOpen)
		*traceReader = TraceReader<L64bu, L64bu>(this->traceFilename);

	// Next, we read the trace and extract the working dataset:
	auto dataset = traceReader->readLines(startLine, endLine);
	BuffersDataset<int> classesDataset;

	if (dataset.accesses.size() > 0) {

		if (this->predictorParams.type == PredictorModelType::BufferSVM) {
			// Now we simulate the buffers and extract the final dataset:
			classesDataset = this->buffersSimulator.simulate(dataset);
		}

		// Finally, we simulate the predictor model and extract metrics from results:
		this->model->importarDatos(dataset, classesDataset);
		resultsAndCosts = this->model->simular();

		if (this->predictorParams.type == PredictorModelType::BufferSVM) {
			BuffersSVMPredictResultsAndCosts* rc = (BuffersSVMPredictResultsAndCosts*)resultsAndCosts.get();
			if (countTotalMemory) {
				rc->cacheMemoryCost = buffersSimulator.historyCache->getTotalMemoryCost();
				rc->dictionaryMemoryCost = buffersSimulator.dictionary.getTotalMemoryCost();
			}
			else {
				rc->cacheMemoryCost = buffersSimulator.historyCache->getMemoryCost();
				rc->dictionaryMemoryCost = buffersSimulator.dictionary.getMemoryCost();
			}
			rc->totalMemoryCost = rc->cacheMemoryCost + rc->dictionaryMemoryCost + rc->modelMemoryCost;

		}
	}
	else this->isNull_ = true;

	/*
	dataset.accesses.clear();
	dataset.accessesInstructions.clear();
	classesDataset.inputAccesses.clear();
	classesDataset.outputAccesses.clear();
	classesDataset.isValid.clear();
	classesDataset.isCacheMiss.clear();
	classesDataset.isDictionaryMiss.clear();
	*/
	// this->model->clean();
	// buffersSimulator.clean();
	dataset = {};
	classesDataset = {};
}

string TracePredictExperiment::getName() {
	return this->traceName;
}

void TracePredictExperiment::clean() {
	// buffersSimulator.clean();
	// model.reset();
	this->model->clean();
	this->buffersSimulator.~BuffersSimulator();
}

PredictorParameters TracePredictExperiment::getPredictorParams() {
	return predictorParams;
}

void TracePredictExperiment::setPredictorParams(PredictorParameters params) {
	this->predictorParams = params;
}