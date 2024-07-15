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

#pragma once
#include "PredictorSVM.h"
#include "TraceReader.h"
#include "Global.h"
#include "BuffersSimulator.h"
#include "PredictorDFCM.h"
#include "PredictorDFCMGradoK.h"

class Experiment {
public:
	virtual long getStartLine() = 0;
	virtual long getEndLine() = 0;
	virtual void setStartLine(long) = 0;
	virtual void setEndLine(long) = 0;
	virtual string getTracefile() = 0;
	virtual void setTraceFile(string traceFilename) = 0;
	virtual string getString() = 0;
	virtual string getName() = 0;
	virtual void setName(string) = 0;
	virtual map<string, double> getResultsAndCosts() = 0;
	virtual void performExperiment() = 0;
	virtual void setPredictorModel(BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>, PredictorSVM<MultiSVMClassifierOneToAll, int>) = 0;
	virtual void setPredictorModel(PredictorDFCMHashOnHash<L64bu, L64b>) = 0;
	virtual void clean() = 0;
	virtual PredictorParameters getPredictorParams() = 0;
	virtual void setPredictorParams(PredictorParameters) = 0;
	virtual bool isNull() = 0;
};


class Experimentation {
public:

	virtual void performExperiments() = 0;
	virtual void exportResults(string filename) = 0;
	virtual vector<Experiment*> getExperiments() = 0;
	virtual void setExperiments(vector<Experiment*>) = 0;
	
};



class TracePredictExperimentation : public Experimentation{
private:
	vector<Experiment*> experiments;
	string outputFilename;
	bool countTotalMemory = false;
	
public:
	static void createAndBuildExperimentations(vector<TracePredictExperimentation>& res, vector<TraceInfo> tracesInfo, PredictorParametersDomain,
		long numAccessesPerExperiment, string outputFilename, bool countTotalMemory);

	static void performAndExportExperimentations(vector<TraceInfo> tracesInfo,
		PredictorParametersDomain params, long numAccessesPerExperiment, string outputFilename, bool countTotalMemory);

	TraceReader<L64bu, L64bu> traceReader;

	TracePredictExperimentation() {
		experiments = vector<Experiment*>();
		outputFilename = "";
	}
	TracePredictExperimentation(vector<Experiment*> experiments, string outputFilename);
	TracePredictExperimentation(string outputFilename, bool countTotalMemory = false);
	
	void performExperiments();
	void exportResults(string filename);
	void exportResults() { exportResults(this->outputFilename); }
	vector<Experiment*> getExperiments();
	void setExperiments(vector<Experiment*>);

	void buildExperiments(vector<TraceInfo> tracesInfo, PredictorParameters params, long numAccessesPerExperiment);

	map<string, vector<Experiment*>> getExperimentsByTrace();

	int numWorkingThreads = 8;
};


class TracePredictExperiment : public Experiment {
private:
	string traceFilename;
	string traceName;
	long startLine;
	long endLine;
	string startDateTime;
	bool countTotalMemory = false;
	bool isNull_ = false;

	BuffersSimulator<L64bu, L64bu, int, L64bu, L64b> buffersSimulator
		 = BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>();
	// PredictorSVM<MultiSVMClassifierOneToAll, int> model;
	shared_ptr<PredictorModel<L64bu, int>> model;
 
	TracePredictExperimentation* framework;
	PredictorParameters predictorParams;

public:
	TracePredictExperiment(TracePredictExperimentation* framework,
		string traceFilename, string traceName, long startLine, long endLine, struct PredictorParameters,
		bool countTotalMemory = false);
	TracePredictExperiment(string traceFilename, string traceName, long startLine, long endLine, struct PredictorParameters,
		bool countTotalMemory = false);

	shared_ptr<PredictResultsAndCosts> resultsAndCosts;


	long getStartLine();
	long getEndLine();
	void setStartLine(long);
	void setEndLine(long);
	string getTracefile();
	void setTraceFile(string traceFilename);
	string getString();
	void setTraceName(string name);
	string getName();
	void setName(string);
	map<string, double> getResultsAndCosts();
	void performExperiment();
	void setPredictorModel(BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>, PredictorSVM<MultiSVMClassifierOneToAll, int>);
	void setPredictorModel(PredictorDFCMHashOnHash<L64bu, L64b>);
	void clean();
	bool isNull();

	PredictorParameters getPredictorParams();
	void setPredictorParams(PredictorParameters);
	
};