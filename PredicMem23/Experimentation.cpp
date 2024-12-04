/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//    Copyright (c) 2024  Pablo Sánchez Cuevas                    //
//                                                                             //
//    This file is part of PredicMem23.                                            //
//                                                                             //
//    PredicMem23 is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by     //
//    the Free Software Foundation, either version 3 of the License, or        //
//    (at your option) any later version.                                      //
//                                                                             //
//    PredicMem23 is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the              //
//    GNU General Public License for more details.                             //
//                                                                             //
//    You should have received a copy of the GNU General Public License        //
//    along with PredicMem23. If not, see <
// http://www.gnu.org/licenses/>.
//
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <time.h>
#include "tinyxml.h"
#include <algorithm> 
#include "Experimentation.h"
#include<omp.h>
#include <filesystem>
namespace fs = std::filesystem;

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

	omp_set_num_threads(this->numWorkingThreads);
	/*
	for (auto& experiment : this->experiments) {
		cout << "\n=========";
		cout << "\nEXPERIMENT: " << experiment->getString() << "\n";
		experiment->performExperiment();
		experiment->clean();
	}
	*/
#pragma omp parallel for schedule(dynamic,1) ordered
	for (int i = 0; i < experiments.size(); i++) {
		auto experiment = this->experiments[i];
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
				if (experiment->getPredictorParams().type == PredictorModelType::BufferSVM) {
					cacheParams_->SetAttribute("numIndexBits", cacheParams.numIndexBits);
					cacheParams_->SetAttribute("numWays", cacheParams.numWays);
					cacheParams_->SetAttribute("numSequenceAccesses", cacheParams.numSequenceAccesses);
					cacheParams_->SetAttribute("saveHistoryAndClassIfNotValid", cacheParams.saveHistoryAndClassIfNotValid);
					experiment_->LinkEndChild(cacheParams_);
				}
				else {
					auto additionalCacheParams = params.additionalCacheParams;
					cacheParams_->SetAttribute("firstTableNumIndexBits", cacheParams.numIndexBits);
					cacheParams_->SetAttribute("secondTableNumIndexBits", additionalCacheParams.numIndexBits);
					cacheParams_->SetAttribute("firstTableNumWays", cacheParams.numWays);
					cacheParams_->SetAttribute("secondTableNumWays", additionalCacheParams.numWays);
					cacheParams_->SetAttribute("numSequenceAccesses", cacheParams.numSequenceAccesses);
					cacheParams_->SetAttribute("saveHistoryAndClassIfNotValid", cacheParams.saveHistoryAndClassIfNotValid);
					experiment_->LinkEndChild(cacheParams_);
				}
				

				// Third node: related to input, dictionary params:
				if (experiment->getPredictorParams().type == PredictorModelType::BufferSVM) {
					auto dictParams = params.dictParams;
					TiXmlElement* dictParams_ = new TiXmlElement("dictParams");
					dictParams_->SetAttribute("numClasses", dictParams.numClasses);
					dictParams_->SetAttribute("maxConfidence", dictParams.maxConfidence);
					dictParams_->SetAttribute("numConfidenceJumps", dictParams.numConfidenceJumps);
					dictParams_->SetAttribute("saveHistoryAndClassIfNotValid", dictParams.saveHistoryAndClassIfNotValid);
					experiment_->LinkEndChild(dictParams_);
				}
			
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
				new TracePredictExperiment(this, filename, name, k, k1, params, this->countTotalMemory));
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

void TracePredictExperimentation::performAndExportExperimentations(string specsFilePath) {
	
	vector<TraceInfo> tracesInfo;
	PredictorParametersDomain params; 
	long numAccessesPerExperiment = 0L; 
	string outputFilename; 
	bool countTotalMemory = false;
	
	TiXmlDocument doc(specsFilePath);
	doc.LoadFile();
	auto root = doc.FirstChildElement();

	for (TiXmlElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement()) {
		string elemName = element->Value();
		if (elemName == "TracesInfo") {
			for (TiXmlElement* traceInfo = element->FirstChildElement(); traceInfo != NULL; traceInfo = traceInfo->NextSiblingElement()) {
				tracesInfo.push_back(decodeTraceInfo(traceInfo));
			}
		}
		else if (elemName == "PredictorParametersDomain") {
			params = decodePredictorParametersDomain(element);
		}
		else if (elemName == "numAccessesPerExperiment") {
			numAccessesPerExperiment = std::stol(element->GetText());
		}
		else if (elemName == "outputFilename") {
			outputFilename = element->GetText();
		}
		else if (elemName == "countTotalMemory") {
			countTotalMemory = (bool)std::stoi(element->GetText());
		}
	}

	if (numAccessesPerExperiment <= 0) {
		string msg = string("ERROR: The number accesses per experiment has to be greater than zero!\n");
		std::cout << msg;
		throw std::invalid_argument(msg);
	}

	TracePredictExperimentation::performAndExportExperimentations(tracesInfo, params, numAccessesPerExperiment, 
		outputFilename, countTotalMemory);
}

void TracePredictExperimentation::performAndExportExperimentations(vector<TraceInfo> tracesInfo,
	PredictorParametersDomain params, long numAccessesPerExperiment, string outputFilename, bool countTotalMemory = false) {
	vector<PredictorParameters> allPredictorParams = decomposePredictorParametersDomain(params);
	int i = 0;
	// std::string baseName = outputFilename.substr(outputFilename.find_last_of("/\\") + 1);
	auto path = fs::path(outputFilename);
	string directory = path.parent_path().string();
	if (directory == "")
		directory = ".";
	std::string baseName = path.stem().string();
	std::string extension = path.extension().string();
	for (auto& predictorParams : allPredictorParams) {
		ostringstream file_;
		file_ << directory << "\\" << baseName << "_" << to_string(i) << extension;
		string file = file_.str();
		// res.push_back(TracePredictExperimentation(file, countTotalMemory));
		auto experimentation = TracePredictExperimentation(file, countTotalMemory);
		experimentation.buildExperiments(tracesInfo, predictorParams, numAccessesPerExperiment);
		experimentation.performExperiments();
		experimentation.exportResults();
		i++;
	}
}

 void TracePredictExperimentation::createAndBuildExperimentations(vector<TracePredictExperimentation>& res, vector<TraceInfo> tracesInfo,
	PredictorParametersDomain params, long numAccessesPerExperiment, string outputFilename, bool countTotalMemory = false) {
	vector<PredictorParameters> allPredictorParams = decomposePredictorParametersDomain(params);
	int i = 0;
	// std::string baseName = outputFilename.substr(outputFilename.find_last_of("/\\") + 1);
	auto path = fs::path(outputFilename);
	string directory = path.parent_path().string();
	if (directory == "")
		directory = ".";
	std::string baseName = path.stem().string();
	std::string extension = path.extension().string();
	res = vector<TracePredictExperimentation>(allPredictorParams.size());
	for (auto& predictorParams : allPredictorParams) {
		ostringstream file_;
		file_ << directory << "\\" << baseName << "_" << to_string(i) << extension;
		string file = file_.str();
		// res.push_back(TracePredictExperimentation(file, countTotalMemory));
		res[i] = TracePredictExperimentation(file, countTotalMemory);
		res[i].buildExperiments(tracesInfo, predictorParams, numAccessesPerExperiment);
		// auto t = TracePredictExperimentation(file, countTotalMemory);
		// t.buildExperiments(tracesInfo, predictorParams, numAccessesPerExperiment);
		// res.push_back(t);
		i++;
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
			new SVM<MultiSVMClassifierOneToAll, int>(cacheParams.numSequenceAccesses, dictParams.numClasses, 
				cacheParams.saveHistoryAndClassIfNotValid));
	}
	else {
		this->model = 
			shared_ptr<PredictorModel<L64bu, int>>((PredictorModel<L64bu, int>*) 
				new HashOnHashDFCM<L64bu, L64b>(cacheType, cacheParams, params.additionalCacheParams));
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
			new SVM<MultiSVMClassifierOneToAll, int>(cacheParams.numSequenceAccesses, dictParams.numClasses, 
				cacheParams.saveHistoryAndClassIfNotValid));
	}
	else {
		if(params.cacheParams.numSequenceAccesses > 0)
			this->model = shared_ptr<PredictorModel<L64bu, int>>(
				(PredictorModel<L64bu, int>*) new KOrderDFCM<L64bu, L64b>(cacheType, cacheParams, params.additionalCacheParams,
					this->countTotalMemory));
		else
			this->model = shared_ptr<PredictorModel<L64bu, int>>(
				(PredictorModel<L64bu, int>*) new HashOnHashDFCM<L64bu, L64b>(cacheType, cacheParams, params.additionalCacheParams,
					this->countTotalMemory));
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
	res << "part_" << this->startDateTime << "__" << traceName << "_" << startLine << "_" << endLine;

	return res.str();
}

void TracePredictExperiment::setTraceName(string name) {
	this->traceName = name;
}

map<string, double> TracePredictExperiment::getResultsAndCosts() {
	return resultsAndCosts->getResultsAndCosts();
}

void TracePredictExperiment::setPredictorModel(BuffersSimulator<L64bu, L64bu, int, L64bu, L64b> bufferSimulator,
	SVM<MultiSVMClassifierOneToAll, int> model) {
	this->buffersSimulator = BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>(bufferSimulator);
	this->model = shared_ptr<PredictorModel<L64bu,int>>((PredictorModel<L64bu, int>*) & model);
}

void TracePredictExperiment::setPredictorModel(HashOnHashDFCM<L64bu, L64b> model) {
	this->model = shared_ptr<PredictorModel<L64bu, int>>((PredictorModel<L64bu, int>*) & model);
}

bool TracePredictExperiment::isNull() {
	return this->isNull_;
}

void TracePredictExperiment::performExperiment() {
	this->startDateTime = nowDateTime();
	AccessesDataset<L64bu, L64bu> dataset;
#pragma omp ordered
	{
	// First, we check that we don't have to instantiate a new TraceReader:
	bool isSameFile = this->framework->traceReader.filename == this->traceFilename;
	bool isFileOpen = this->framework->traceReader.file.is_open();

	if (!isSameFile || !isFileOpen) {
		// this->framework->traceReader = TraceReader<L64bu, L64bu>(this->traceFilename);
		
		// this->framework->traceReader.file = ifstream(this->traceFilename);
		// this->framework->traceReader.filename = this->traceFilename;
		// this->framework->traceReader.file.open(this->traceFilename);
		TraceReader<L64bu, L64bu>(this->traceFilename).copy(this->framework->traceReader);
	}

	// Next, we read the trace and extract the working dataset:
	dataset = this->framework->traceReader.readLines(startLine, endLine);
	}
	BuffersDataset<int> classesDataset;

	if (dataset.accesses.size() > 0) {

		if (this->predictorParams.type == PredictorModelType::BufferSVM) {
			// Now we simulate the buffers and extract the final dataset:
			classesDataset = this->buffersSimulator.simulate(dataset);
		}

		// Finally, we simulate the predictor model and extract metrics from results:
		this->model->importData(dataset, classesDataset);
		resultsAndCosts = this->model->simulate();

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