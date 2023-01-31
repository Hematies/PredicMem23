#include "Experimentation.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <time.h>
#include "tinyxml.h"
#include <algorithm> 


TracePredictExperientation::TracePredictExperientation(vector<shared_ptr<Experiment>> experiments, string outputFilename) {
	// this->experiments = vector<shared_ptr<Experiment>>(experiments);
	this->experiments = experiments;
	this->outputFilename = outputFilename;
	this->traceReader = TraceReader<L64b, L64b>();
}

TracePredictExperientation::TracePredictExperientation(string outputFilename) {
	TracePredictExperientation(vector<shared_ptr<Experiment>>(), outputFilename);
}

void TracePredictExperientation::performExperiments() {
	for (auto& experiment : this->experiments) {
		experiment->performExperiment();
	}
}

vector<shared_ptr<Experiment>> TracePredictExperientation::getExperiments() {
	// return vector<shared_ptr<Experiment>>(experiments);
	return experiments;
}

void TracePredictExperientation::setExperiments(vector<shared_ptr<Experiment>> experiments) {
	// this->experiments = vector<shared_ptr<Experiment>>(experiments);
	this->experiments = experiments;
}

map<string, vector<shared_ptr<Experiment>>> TracePredictExperientation::getExperimentsByTrace() {
	map<string, vector<shared_ptr<Experiment>>> res = map<string, vector<shared_ptr<Experiment>>>();
	for (auto& experiment : this->experiments) {
		string traceName = experiment->getName();
		
		if (res.find(traceName) == res.end())
			res[traceName] = vector<shared_ptr<Experiment>>();

		res[traceName].push_back(experiment);
	}
	for (auto iter = res.begin(); iter != res.end(); ++iter) {
		string traceName = iter->first;
		std::sort(res[traceName].begin(), res[traceName].end(),
			[](shared_ptr<Experiment>& experiment1, shared_ptr<Experiment>& experiment2) -> bool
			{
				return experiment1->getStartLine() < experiment2->getStartLine();
			});
	}
	return res;
}

void TracePredictExperientation::exportResults(string filename) {
	
	TiXmlDocument doc;
	TiXmlDeclaration decl("1.0", "", "");
	doc.InsertEndChild(decl);

	TiXmlElement traces("Traces");
	doc.LinkEndChild(&traces);

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
			experiment_->SetDoubleAttribute("hitRate", results["hitRate"]);
			experiment_->SetDoubleAttribute("cacheMissRate", results["cacheMissRate"]);
			experiment_->SetDoubleAttribute("dictionaryMissRate", results["dictionaryMissRate"]);
			trace->LinkEndChild(experiment_); // OJO CON LOS PUNTEROS

			totalResults.hitRate += results["hitRate"] / numExperiments;
			totalResults.cacheMissRate += results["cacheMissRate"] / numExperiments;
			totalResults.dictionaryMissRate += results["dictionaryMissRate"] / numExperiments;
			// totalResults.modelMemoryCosts += results["modelMemoryCosts"];
			// totalResults.dictionaryMemoryCosts += results["dictionaryMemoryCosts"];
			// totalResults.cacheMemoryCosts += results["cacheMemoryCosts"];
		}

		trace->SetDoubleAttribute("hitRate", totalResults.hitRate);
		trace->SetDoubleAttribute("cacheMissRate", totalResults.cacheMissRate);
		trace->SetDoubleAttribute("dictionaryMissRate", totalResults.dictionaryMissRate);
		traces.LinkEndChild(trace);
	}

	doc.SaveFile(filename.c_str());
}

void TracePredictExperientation::buildExperiments(vector<string> names, vector<string> filenames, 
	PredictorParameters params, long numAccessesPerExperiment = 10000000) {

	auto pointer = shared_ptr<TracePredictExperientation>(this);
	for (int i = 0; i < names.size(); i++) {
		auto name = names[i];
		auto filename = filenames[i];

		// auto experiment = TracePredictExperiment(this, filename, name, )
		TraceReader<L64b, L64b> reader(filename);
		unsigned long numLines = reader.countNumLines();
		unsigned long k = 0;
		unsigned long k1 = numAccessesPerExperiment;
		while(true) {
			k1 = k1 > numLines ? numLines : k1;
			auto experiment = TracePredictExperiment(pointer, filename, name, k, k1, params);
			this->experiments.push_back(shared_ptr<TracePredictExperiment>(&experiment));
			// this->experiments.push_back(make_unique<TracePredictExperiment>(experiment));

			k += numAccessesPerExperiment;
			k1 += numAccessesPerExperiment;
			if (k > numLines) break;
		}

	}
}

TracePredictExperiment::TracePredictExperiment(string traceFilename, string traceName, long startLine, long endLine, 
	struct PredictorParameters params) {
	this->traceFilename = traceFilename;
	this->traceName = traceName;
	this->startLine = startLine;
	this->endLine = endLine;

	auto cacheParams = params.cacheParams;
	auto dictParams = params.dictParams;
	
	HistoryCacheType cacheType = (cacheParams.numIndexBits > 0)? HistoryCacheType::Real : HistoryCacheType::Infinite;

	this->buffersSimulator = BuffersSimulator<L64b, L64b, int, L64b>(cacheType, cacheParams.numSequenceAccesses, dictParams.numClasses,
		dictParams.maxConfidence, dictParams.numConfidenceJumps, dictParams.saveHistoryAndClassAfterMiss);

	this->model = PredictorSVM<MultiSVMClassifierOneToAll, int>(cacheParams.numSequenceAccesses, dictParams.numClasses);
}

TracePredictExperiment::TracePredictExperiment(shared_ptr<TracePredictExperientation> framework, string traceFilename, string traceName, long startLine, long endLine,
	struct PredictorParameters params) {
	this->framework = framework;
	TracePredictExperiment(traceFilename, traceName, startLine, endLine,
		params);
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
	// auto now = std::chrono::system_clock::now();
	auto t = std::time(nullptr);
	tm tm;
	localtime_s(&tm, &t);
	std::ostringstream oss; 
	oss << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");
	string now = oss.str();

	ostringstream res; 
	res << now << "\\" << traceName << "_" << startLine << "_" << endLine;

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
	this->buffersSimulator = bufferSimulator;
	this->model = model;
}

void TracePredictExperiment::performExperiment() {
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
}

string TracePredictExperiment::getName() {
	return this->traceName;
}