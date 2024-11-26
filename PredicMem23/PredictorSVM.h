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
#include <string>
#include <iostream>
#include "SVMClassifier.hpp"
#include "BuffersSimulator.h"
//#include "Experimentation.h"
#include "Global.h"
#include "PredictorModel.h"

using namespace std;



template<typename T_pred, typename T_input>
class SVM : PredictorModel<L64bu, T_input>
{

private:
	int numPartsToPrint = 10000;
	int numInputClasses;
public:
	vector<vector<float>> inputData = vector<vector<float>>();
	vector<char> outputData = vector<char>();
	vector<char> predictableInputsMask = vector<char>();
	vector<char> inputBufferMissesMask = vector<char>();
	vector<char> dictionaryMissesMask = vector<char>();

	long numHits = 0;
	int numBatchSamples = 1;
	int numRepetitions = 1;
	double hitRate = 0.0;
	T_pred model;

	int numSequenceElements = 0;
	int numClasses = 0;

	bool predictOnNonValidInput;
	
	~SVM() {
		clean();
	}
	
	void clean() {
		
		this->inputData = vector<vector<float>>();
		this->outputData = vector<char>();
		this->predictableInputsMask = vector<char>();
		this->inputBufferMissesMask = vector<char>();
		this->dictionaryMissesMask = vector<char>();
	}

	SVM(BuffersDataset<T_input> classesDataset, int numSequenceElements, int numClasses, bool predictOnNonValidInput) {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Class is not sub-type of MultiSVMClassifier");

		this->numSequenceElements = numSequenceElements;
		this->numClasses = numClasses;
		this->predictOnNonValidInput = predictOnNonValidInput;
		if (predictOnNonValidInput) numInputClasses++;


		importData(classesDataset);
		initializeModel();
	}

	SVM(int numSequenceElements, int numClasses, bool predictOnNonValidInput) {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Class is not sub-type of MultiSVMClassifier");

		this->numSequenceElements = numSequenceElements;
		this->numClasses = numClasses;
		this->predictOnNonValidInput = predictOnNonValidInput;
		if (predictOnNonValidInput) numInputClasses++;


		initializeModel();
	}

	SVM() {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Class is not sub-type of MultiSVMClassifier");

		this->numSequenceElements = 0;
		this->numClasses = 0;
		this->predictOnNonValidInput = true;
		
	}

	void importData(AccessesDataset<L64bu, L64bu>& data, BuffersDataset<T_input>& classesDataset) {
		importData(classesDataset);
	}

	void importData(BuffersDataset<T_input>& classesDataset) {
		for (int i = 0; i < classesDataset.inputAccesses.size(); i++) {
			vector<float> input = vector<float>();
			char output = -1;
			char isInputValid = false;

			for (int j = 0; j < classesDataset.inputAccesses[i].size(); j++) {
				input.push_back(((float)classesDataset.inputAccesses[i][j]) / numInputClasses + 1.0);
			}

			output = classesDataset.outputAccesses[i];
			isInputValid = classesDataset.isValid[i];

			this->inputData.push_back(input);
			this->outputData.push_back(output);
			this->predictableInputsMask.push_back(isInputValid);
			this->inputBufferMissesMask.push_back(classesDataset.isCacheMiss[i]);
			this->dictionaryMissesMask.push_back(classesDataset.isDictionaryMiss[i]);
		}
	}

	void initializeModel() {
		double c = 1.0;
		double learningRate = 0.7;
		model = T_pred(this->numSequenceElements, this->numClasses, c, 1, learningRate);
	}

	void fit(vector<float> input, int output) {
		auto in = vector<vector<double>>{ vector<double>(input.begin(), input.end()) };
		auto out = vector<int>{ output };
		this->model.fit(in, out);
	}

	int predict(vector<float> input) {
		auto in = vector<vector<double>>{ vector<double>(input.begin(), input.end()) };
		return this->model.predict(in)[0];
	}

	shared_ptr<PredictResultsAndCosts> simulate(bool initialize = true) {

		BuffersSVMPredictResultsAndCosts resultsAndCosts = BuffersSVMPredictResultsAndCosts();
		double numDictionaryMisses = 0.0;
		double numCacheMisses = 0.0;


		if (initialize) {
			this->initializeModel();

		}

		numHits = 0;
		hitRate = 0.0;

		for (int i = 0; i < inputData.size(); i++) {
			vector<float> input = vector<float>(inputData[i].begin(), inputData[i].end());
			int output = outputData[i];
			auto isInputPredictable = predictableInputsMask[i];
			auto inputBufferMiss = inputBufferMissesMask[i];
			auto dictionaryMiss = dictionaryMissesMask[i];

			int predictedOutput = -1;
			if(isInputPredictable)
				predictedOutput = predict(input);

			bool predictionMiss = (output != predictedOutput);

			// If thre was a miss, the fitting is performed with the input and output sample:
			bool performFitting = !inputBufferMiss && (predictionMiss || dictionaryMiss);
			if (performFitting) {
				fit(input, output);
			}
			
			bool hit = isInputPredictable && !dictionaryMiss && !inputBufferMiss && !predictionMiss;
			if (hit)
				numHits++;

			if (dictionaryMiss) numDictionaryMisses++;
			if (inputBufferMiss) numCacheMisses++;
				

			if (i % numPartsToPrint == 0) {
				string in = "";
				for (auto e : input)
					in += to_string((e - 1.0) * numInputClasses) + ", ";
				std::cout << in << " -> " << output << " vs " << predictedOutput << std::endl;
				std::cout << "Tasa de éxito: " << (double)numHits / (i + 1) << " ; " << ((double)i) / inputData.size() << std::endl;
			}

		}

		hitRate = ((double)numHits) / inputData.size();

		resultsAndCosts.hitRate = hitRate;
		resultsAndCosts.dictionaryMissRate = numDictionaryMisses / inputData.size();
		resultsAndCosts.cacheMissRate = numCacheMisses / inputData.size();
		resultsAndCosts.modelMemoryCost = getModelMemoryCosts();
		return shared_ptr<PredictResultsAndCosts>((PredictResultsAndCosts*) new BuffersSVMPredictResultsAndCosts(resultsAndCosts));
	}

	double getModelMemoryCosts() {
		int numSVMs = this->model.SVMsTable.size();
		int numElements = this->model.numFeatures + 1;

		return numElements * sizeof(float) * numSVMs; 
			// For now, we return the total number bytes of the elements (weights) of the model.
	}
};


