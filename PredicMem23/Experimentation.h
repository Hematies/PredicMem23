/////////////////////////////////////////////////////////////////////////////////
/// @file Experimentation.h
/// @brief Declaration of classes for running experiments on predictive memory systems.
///
/// This file contains the declarations of classes and functions used to set up and run
/// experiments for evaluating predictive memory systems.
///
/// @details
/// The following classes and functions are included:
/// - Experiment
/// - Experimentation
/// - TracePredictExperimentation
/// - TracePredictExperiment
///
/// @section LICENSE
/// Copyright (c) 2024 Pablo Sánchez Cuevas
///
/// This file is part of PredicMem23.
///
/// PredicMem23 is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// PredicMem23 is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with PredicMem23. If not, see <http://www.gnu.org/licenses/>.
///
/// @section DESCRIPTION
/// This file declares the classes and methods for setting up and running experiments
/// to evaluate predictive memory systems. It includes classes for defining individual
/// experiments and collections of experiments, as well as methods for performing
/// and exporting results.
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "PredictorSVM.h"
#include "TraceReader.h"
#include "Global.h"
#include "BuffersSimulator.h"
#include "DFCM.h"
#include "KOrderDFCM.h"

/**
 * @brief Abstract class representing a single experiment.
 */
class Experiment {
public:
	/**
	 * @brief Get the start line of the experiment.
	 * @return The start line.
	 */
	virtual long getStartLine() = 0;
	/**
	* @brief Get the end line of the experiment.
	* @return The end line.
	*/
	virtual long getEndLine() = 0;
	/**
	 * @brief Set the start line of the experiment.
	 * @param startLine The start line to set.
	 */
	virtual void setStartLine(long) = 0;
	/**
	 * @brief Set the end line of the experiment.
	 * @param endLine The end line to set.
	 */
	virtual void setEndLine(long) = 0;
	/**
	 * @brief Get the trace file used in the experiment.
	 * @return The trace file name.
	 */
	virtual string getTracefile() = 0;
	/**
	 * @brief Set the trace file used in the experiment.
	 * @param traceFilename The trace file name to set.
	 */
	virtual void setTraceFile(string traceFilename) = 0;
	/**
	 * @brief Get a string representation of the experiment.
	 * @return A string representation of the experiment.
	 */
	virtual string getString() = 0;
	/**
	 * @brief Get the name of the experiment.
	 * @return The name of the experiment.
	 */
	virtual string getName() = 0;
	/**
	 * @brief Set the name of the experiment.
	 * @param name The name to set.
	 */
	virtual void setName(string) = 0;
	/**
	 * @brief Get the results and costs of the experiment.
	 * @return A map of results and costs.
	 */
	virtual map<string, double> getResultsAndCosts() = 0;
	/**
	 * @brief Perform the experiment.
	 */
	virtual void performExperiment() = 0;
	/**
	 * @brief Set the predictor model using a buffer simulator and SVM predictor.
	 * @param bufferSimulator The buffer simulator.
	 * @param model The SVM predictor model.
	 */
	virtual void setPredictorModel(BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>, SVM<MultiSVMClassifierOneToAll, int>) = 0;
	/**
	 * @brief Set the predictor model using a DFCM predictor.
	 * @param model The DFCM predictor model.
	 */
	virtual void setPredictorModel(HashOnHashDFCM<L64bu, L64b>) = 0;
	/**
	 * @brief Clean the experiment.
	 */
	virtual void clean() = 0;
	/**
	 * @brief Get the predictor parameters.
	 * @return The predictor parameters.
	 */
	virtual PredictorParameters getPredictorParams() = 0;
	/**
	* @brief Set the predictor parameters.
	* @param params The predictor parameters to set.
	*/
	virtual void setPredictorParams(PredictorParameters) = 0;
	/**
	 * @brief Check if the experiment is null.
	 * @return True if the experiment is null, false otherwise.
	 */
	virtual bool isNull() = 0;
};


/**
 * @brief Abstract class representing a collection of experiments.
 */
class Experimentation {
public:

	/**
	 * @brief Perform all experiments in the collection.
	 */
	virtual void performExperiments() = 0;
	/**
	 * @brief Export the results of all experiments to a file.
	 * @param filename The name of the file to export results to.
	 */
	virtual void exportResults(string filename) = 0;
	/**
	 * @brief Get the experiments in the collection.
	 * @return A vector of pointers to experiments.
	 */
	virtual vector<Experiment*> getExperiments() = 0;
	/**
	* @brief Set the experiments in the collection.
	* @param experiments A vector of pointers to experiments.
	*/
	virtual void setExperiments(vector<Experiment*>) = 0;
	
};



/**
 * @brief Class representing a collection of trace prediction experiments.
 */
class TracePredictExperimentation : public Experimentation{
private:
	vector<Experiment*> experiments; ///< Vector of experiments.
	string outputFilename; ///< Output filename for results.
	bool countTotalMemory = false; ///< Flag to indicate if total memory should be counted.

public:
	/**
	* @brief Static method to create and build experimentations.
	* @param res Vector to store the created TracePredictExperimentation objects.
	* @param tracesInfo Vector of trace information.
	* @param params Predictor parameters domain.
	* @param numAccessesPerExperiment Number of accesses per experiment.
	* @param outputFilename Output filename for results.
	* @param countTotalMemory Flag to indicate if total memory should be counted.
	*/
	static void createAndBuildExperimentations(vector<TracePredictExperimentation>& res, vector<TraceInfo> tracesInfo, PredictorParametersDomain,
		long numAccessesPerExperiment, string outputFilename, bool countTotalMemory);

	/**
	 * @brief Static method to perform and export experimentations.
	 * @param tracesInfo Vector of trace information.
	 * @param params Predictor parameters domain.
	 * @param numAccessesPerExperiment Number of accesses per experiment.
	 * @param outputFilename Output filename for results.
	 * @param countTotalMemory Flag to indicate if total memory should be counted.
	 */
	static void performAndExportExperimentations(vector<TraceInfo> tracesInfo,
		PredictorParametersDomain params, long numAccessesPerExperiment, string outputFilename, bool countTotalMemory);

	TraceReader<L64bu, L64bu> traceReader; ///< Trace reader.

	/**
	 * @brief Default constructor.
	 */
	TracePredictExperimentation() {
		experiments = vector<Experiment*>();
		outputFilename = "";
	}
	/**
	 * @brief Constructor with experiments and output filename.
	 * @param experiments Vector of experiments.
	 * @param outputFilename Output filename for results.
	 */
	TracePredictExperimentation(vector<Experiment*> experiments, string outputFilename);
	/**
	 * @brief Constructor with output filename and total memory flag.
	 * @param outputFilename Output filename for results.
	 * @param countTotalMemory Flag to indicate if total memory should be counted.
	 */
	TracePredictExperimentation(string outputFilename, bool countTotalMemory = false);
	
	/**
	 * @brief Perform all experiments in the collection.
	 */
	void performExperiments();
	/**
	 * @brief Export the results of all experiments to a file.
	 * @param filename The name of the file to export results to.
	 */
	void exportResults(string filename);
	/**
	 * @brief Export the results of all experiments to the default file.
	 */
	void exportResults() { exportResults(this->outputFilename); }
	/**
	* @brief Get the experiments in the collection.
	* @return A vector of pointers to experiments.
	*/
	vector<Experiment*> getExperiments();
	/**
	 * @brief Set the experiments in the collection.
	 * @param experiments A vector of pointers to experiments.
	 */
	void setExperiments(vector<Experiment*>);

	/**
	 * @brief Build experiments based on trace information and predictor parameters.
	 * @param tracesInfo Vector of trace information.
	 * @param params Predictor parameters.
	 * @param numAccessesPerExperiment Number of accesses per experiment.
	 */
	void buildExperiments(vector<TraceInfo> tracesInfo, PredictorParameters params, long numAccessesPerExperiment);

	/**
	 * @brief Get experiments grouped by trace.
	 * @return A map with trace names as keys and vectors of experiments as values.
	 */
	map<string, vector<Experiment*>> getExperimentsByTrace();
	 
	int numWorkingThreads = 8; ///< Number of working threads.
};


/**
 * @brief Class representing a single trace prediction experiment.
 */
class TracePredictExperiment : public Experiment {
private:
	string traceFilename; ///< Trace filename.
	string traceName; ///< Trace name.
	long startLine; ///< Start line.
	long endLine; ///< End line.
	string startDateTime; ///< Start date and time.
	bool countTotalMemory = false; ///< Flag to indicate if total memory should be counted.
	bool isNull_ = false; ///< Flag to indicate if the experiment is null.

	BuffersSimulator<L64bu, L64bu, int, L64bu, L64b> buffersSimulator = BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>();
	shared_ptr<PredictorModel<L64bu, int>> model; ///< Predictor model.
	// SVM<MultiSVMClassifierOneToAll, int> model;

	TracePredictExperimentation* framework; ///< Pointer to the experimentation framework.
	PredictorParameters predictorParams; ///< Predictor parameters.

public:
	/**
	 * @brief Constructor with parameters.
	 * @param framework Pointer to the experimentation framework.
	 * @param traceFilename Trace filename.
	 * @param traceName Trace name.
	 * @param startLine Start line.
	 * @param endLine End line.
	 * @param params Predictor parameters.
	 * @param countTotalMemory Flag to indicate if total memory should be counted.
	 */
	TracePredictExperiment(TracePredictExperimentation* framework,
		string traceFilename, string traceName, long startLine, long endLine, struct PredictorParameters,
		bool countTotalMemory = false);
	/**
	 * @brief Constructor with parameters.
	 * @param traceFilename Trace filename.
	 * @param traceName Trace name.
	 * @param startLine Start line.
	 * @param endLine End line.
	 * @param params Predictor parameters.
	 * @param countTotalMemory Flag to indicate if total memory should be counted.
	 */
	TracePredictExperiment(string traceFilename, string traceName, long startLine, long endLine, struct PredictorParameters,
		bool countTotalMemory = false);

	shared_ptr<PredictResultsAndCosts> resultsAndCosts; ///< Shared pointer to results and costs.

	/**
	 * @brief Get the start line of the experiment.
	 * @return The start line.
	 */
	long getStartLine();
	/**
	 * @brief Get the end line of the experiment.
	 * @return The end line.
	 */
	long getEndLine();
	/**
	 * @brief Set the start line of the experiment.
	 * @param startLine The start line to set.
	 */
	void setStartLine(long);
	/**
	* @brief Set the end line of the experiment.
	* @param endLine The end line to set.
	*/
	void setEndLine(long);
	/**
	 * @brief Get the trace file used in the experiment.
	 * @return The trace file name.
	 */
	string getTracefile();
	/**
	* @brief Set the trace file used in the experiment.
	* @param traceFilename The trace file name to set.
	*/
	void setTraceFile(string traceFilename);
	/**
	 * @brief Get a string representation of the experiment.
	 * @return A string representation of the experiment.
	 */
	string getString();
	/**
	 * @brief Set the trace name of the experiment.
	 * @param name The trace name to set.
	 */
	void setTraceName(string name);
	/**
	 * @brief Get the name of the experiment.
	 * @return The name of the experiment.
	 */
	string getName();
	/**
	 * @brief Set the name of the experiment.
	 * @param name The name to set.
	 */
	void setName(string);
	/**
	* @brief Get the results and costs of the experiment.
	* @return A map of results and costs.
	*/
	map<string, double> getResultsAndCosts();
	/**
	* @brief Perform the experiment.
	*/
	void performExperiment();
	/**
	 * @brief Set the predictor model using a buffer simulator and SVM predictor.
	 * @param bufferSimulator The buffer simulator.
	 * @param model The SVM predictor model.
	 */
	void setPredictorModel(BuffersSimulator<L64bu, L64bu, int, L64bu, L64b>, SVM<MultiSVMClassifierOneToAll, int>);
	/**
	 * @brief Set the predictor model using a DFCM predictor.
	 * @param model The DFCM predictor model.
	 */
	void setPredictorModel(HashOnHashDFCM<L64bu, L64b>);
	/**
	 * @brief Clean the experiment.
	 */
	void clean();
	/**
	 * @brief Check if the experiment is null.
	 * @return True if the experiment is null, false otherwise.
	 */
	bool isNull();

	/**
	 * @brief Get the predictor parameters.
	 * @return The predictor parameters.
	 */
	PredictorParameters getPredictorParams();
	/**
	 * @brief Set the predictor parameters.
	 * @param params The predictor parameters to set.
	 */
	void setPredictorParams(PredictorParameters);
	
};