#pragma once
#include "PredictorSVM.h"
#include "TraceReader.h"
#include "Global.h"
#include "BuffersSimulator.h"

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
	virtual map<string, double> getResults() = 0;
	virtual void performExperiment() = 0;
	virtual void setPredictor(BuffersSimulator<L64b, L64b, int, L64b>, PredictorSVM<MultiSVMClassifierOneToAll, int>) = 0;
	virtual void clean() = 0;
	virtual PredictorParameters getPredictorParams() = 0;
	virtual void setPredictorParams(PredictorParameters) = 0;
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
	
public:
	TraceReader<L64b, L64b> traceReader;

	TracePredictExperimentation(vector<Experiment*> experiments, string outputFilename);
	TracePredictExperimentation(string outputFilename);

	void performExperiments();
	void exportResults(string filename);
	void exportResults() { exportResults(this->outputFilename); }
	vector<Experiment*> getExperiments();
	void setExperiments(vector<Experiment*>);

	void buildExperiments(vector<TraceInfo> tracesInfo, PredictorParameters params, long numAccessesPerExperiment);

	map<string, vector<Experiment*>> getExperimentsByTrace();
};

class TracePredictExperiment : public Experiment {
private:
	string traceFilename;
	string traceName;
	long startLine;
	long endLine;
	string startDateTime;

	BuffersSimulator<L64b, L64b, int, L64b> buffersSimulator
		 = BuffersSimulator<L64b, L64b, int, L64b>();
	PredictorSVM<MultiSVMClassifierOneToAll, int> model;

	TracePredictExperimentation* framework;
	PredictorParameters predictorParams;

public:
	TracePredictExperiment(TracePredictExperimentation* framework,
		string traceFilename, string traceName, long startLine, long endLine, struct PredictorParameters);
	TracePredictExperiment(string traceFilename, string traceName, long startLine, long endLine, struct PredictorParameters);

	PredictResultsAndCosts resultsAndCosts;


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
	map<string, double> getResults();
	void performExperiment();
	void setPredictor(BuffersSimulator<L64b, L64b, int, L64b>, PredictorSVM<MultiSVMClassifierOneToAll, int>);
	void clean();

	PredictorParameters getPredictorParams();
	void setPredictorParams(PredictorParameters);
	
};