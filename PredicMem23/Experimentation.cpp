
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
	this->traceReader = TraceReader<L64b, L64b>();
	
}

TracePredictExperimentation::TracePredictExperimentation(string outputFilename) {
	this->experiments = vector<Experiment*>();
	this->outputFilename = outputFilename;
	this->traceReader = TraceReader<L64b, L64b>();

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
		PredictResultsAndCosts totalResults{
			0.0,
			0.0,
			0.0,
			0.0,
			0.0,
			0.0
		};

		int numExperiments = experimentsByTrace[traceName].size();
		for (auto experiment : experimentsByTrace[traceName]) {
			TiXmlElement* experiment_ = new TiXmlElement(experiment->getString().c_str());
			auto results = experiment->getResults();
			auto params = experiment->getPredictorParams();

			// First node: related to results:
			TiXmlElement* results_ = new TiXmlElement("results");
			results_->SetDoubleAttribute("hitRate", results["hitRate"]);
			results_->SetDoubleAttribute("cacheMissRate", results["cacheMissRate"]);
			results_->SetDoubleAttribute("dictionaryMissRate", results["dictionaryMissRate"]);
			experiment_->LinkEndChild(results_);

			// Second node: related to input, cache params:
			auto cacheParams = params.cacheParams;
			TiXmlElement* cacheParams_ = new TiXmlElement("cacheParams");
			cacheParams_->SetAttribute("numIndexBits", cacheParams.numIndexBits);
			cacheParams_->SetAttribute("numWays", cacheParams.numWays);
			cacheParams_->SetAttribute("numSequenceAccesses", cacheParams.numSequenceAccesses);
			experiment_->LinkEndChild(cacheParams_);

			// Third node: related to input, dictionary params:
			auto dictParams = params.dictParams;
			TiXmlElement* dictParams_ = new TiXmlElement("dictParams");
			dictParams_->SetAttribute("numClasses", dictParams.numClasses);
			dictParams_->SetAttribute("numEntries", dictParams.numEntries);
			dictParams_->SetAttribute("maxConfidence", dictParams.maxConfidence);
			dictParams_->SetAttribute("numConfidenceJumps", dictParams.numConfidenceJumps);
			dictParams_->SetAttribute("saveHistoryAndClassAfterMiss", dictParams.saveHistoryAndClassAfterMiss);
			experiment_->LinkEndChild(dictParams_);

			// Fourth node: related to input, model params: TODO
			
			trace->LinkEndChild(experiment_);
			totalResults.hitRate += results["hitRate"] / numExperiments;
			totalResults.cacheMissRate += results["cacheMissRate"] / numExperiments;
			totalResults.dictionaryMissRate += results["dictionaryMissRate"] / numExperiments;
			// totalResults.modelMemoryCost += results["modelMemoryCost"];
			// totalResults.dictionaryMemoryCost += results["dictionaryMemoryCost"];
			// totalResults.cacheMemoryCost += results["cacheMemoryCost"];
		}

		trace->SetDoubleAttribute("hitRate", totalResults.hitRate);
		trace->SetDoubleAttribute("cacheMissRate", totalResults.cacheMissRate);
		trace->SetDoubleAttribute("dictionaryMissRate", totalResults.dictionaryMissRate);
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
		TraceReader<L64b, L64b> reader(filename);
		unsigned long numLines = tracesInfo[i].numAccesses;
		unsigned long k = 0;
		unsigned long k1 = numAccessesPerExperiment;
		while(true) {
			k1 = k1 > numLines ? numLines : k1;
			this->experiments.push_back(
				new TracePredictExperiment(pointer, filename, name, k, k1, params));
			// this->experiments.push_back(make_unique<TracePredictExperiment>(experiment));

			k += numAccessesPerExperiment;
			k1 += numAccessesPerExperiment;
			if (k >= numLines) break;
		}

	}
}

TracePredictExperiment::TracePredictExperiment(string traceFilename, string traceName, long startLine, long endLine, 
	struct PredictorParameters params) {
	this->traceFilename = traceFilename;
	this->traceName = traceName;
	this->startLine = startLine;
	this->endLine = endLine;

	this->predictorParams = params;
	auto cacheParams = params.cacheParams;
	auto dictParams = params.dictParams;
	
	HistoryCacheType cacheType = (cacheParams.numIndexBits > 0)? HistoryCacheType::Real : HistoryCacheType::Infinite;

	this->buffersSimulator = BuffersSimulator<L64b, L64b, int, L64b>(cacheType, cacheParams, dictParams);

	this->model = PredictorSVM<MultiSVMClassifierOneToAll, int>(cacheParams.numSequenceAccesses, dictParams.numClasses);

	this->startDateTime = nowDateTime();
}

TracePredictExperiment::TracePredictExperiment(TracePredictExperimentation* framework, string traceFilename, string traceName, long startLine, long endLine,
	struct PredictorParameters params) {
	this->framework = framework;
	this->traceFilename = traceFilename;
	this->traceName = traceName;
	this->startLine = startLine;
	this->endLine = endLine;

	this->predictorParams = params;
	auto cacheParams = params.cacheParams;
	auto dictParams = params.dictParams;

	HistoryCacheType cacheType = (cacheParams.numIndexBits > 0) ? HistoryCacheType::Real : HistoryCacheType::Infinite;

	this->buffersSimulator = BuffersSimulator<L64b, L64b, int, L64b>(cacheType, cacheParams, dictParams);

	this->model = PredictorSVM<MultiSVMClassifierOneToAll, int>(cacheParams.numSequenceAccesses, dictParams.numClasses);

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

map<string, double> TracePredictExperiment::getResults() {
	auto res = map<string, double>{
		{"hitRate", resultsAndCosts.hitRate},
		{"cacheMissRate", resultsAndCosts.cacheMissRate},
		{"dictionaryMissRate", resultsAndCosts.dictionaryMissRate}
	};
	return res;
}

void TracePredictExperiment::setPredictor(BuffersSimulator<L64b, L64b, int, L64b> bufferSimulator,
	PredictorSVM<MultiSVMClassifierOneToAll, int> model) {
	this->buffersSimulator = BuffersSimulator<L64b, L64b, int, L64b>(bufferSimulator);
	this->model = model;
}

void TracePredictExperiment::performExperiment() {
	this->startDateTime = nowDateTime();

	// First, we check that we don't have to instantiate a new TraceReader:
	TraceReader<L64b, L64b>* traceReader = &this->framework->traceReader;
	bool isSameFile = traceReader->filename == this->traceFilename;
	bool isFileOpen = traceReader->file.is_open();

	if (!isSameFile || !isFileOpen)
		*traceReader = TraceReader<L64b, L64b>(this->traceFilename);

	// Next, we read the trace and extract the working dataset:
	auto dataset = traceReader->readLines(startLine, endLine);

	// Now we simulate the buffers and extract the final dataset:
	BuffersDataset<int> classesDataset = this->buffersSimulator.simulate(dataset);

	// Finally, we simulate the predictor model and extract metrics from results:
	this->model.importarDatos(classesDataset);
	resultsAndCosts = this->model.simular();

	dataset.accesses.clear();
	dataset.accessesInstructions.clear();
	classesDataset.inputAccesses.clear();
	classesDataset.outputAccesses.clear();
	classesDataset.isValid.clear();
	classesDataset.isCacheMiss.clear();
	classesDataset.isDictionaryMiss.clear();
}

string TracePredictExperiment::getName() {
	return this->traceName;
}

void TracePredictExperiment::clean() {
	buffersSimulator.clean();
	model.~PredictorSVM();

}

PredictorParameters TracePredictExperiment::getPredictorParams() {
	return predictorParams;
}

void TracePredictExperiment::setPredictorParams(PredictorParameters params) {
	this->predictorParams = params;
}