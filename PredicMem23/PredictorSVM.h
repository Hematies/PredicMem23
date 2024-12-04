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

/**
 * @brief Template class for a Support Vector Machine (SVM) model used as a predictor.
 *
 * This class represents an SVM-based predictor that can be trained and used to make predictions on input data.
 * It leverages a multi-class SVM classifier and uses buffers and datasets for training and testing.
 *
 * @tparam T_pred The type of the SVM classifier.
 * @tparam T_input The type of input data used for prediction and training.
 */
template<typename T_pred, typename T_input>
class SVM : PredictorModel<L64bu, T_input> {

private:
    int numPartsToPrint = 10000; ///< Number of parts to print during simulation.
    int numInputClasses; ///< The number of input classes.

public:
    vector<vector<float>> inputData = vector<vector<float>>(); ///< Input data for training.
    vector<char> outputData = vector<char>(); ///< Output data for training.
    vector<char> predictableInputsMask = vector<char>(); ///< Mask for predictable inputs.
    vector<char> inputBufferMissesMask = vector<char>(); ///< Mask for input buffer misses.
    vector<char> dictionaryMissesMask = vector<char>(); ///< Mask for dictionary misses.

    long numHits = 0; ///< Number of hits during simulation.
    int numBatchSamples = 1; ///< Number of batch samples.
    int numRepetitions = 1; ///< Number of repetitions during simulation.
    double hitRate = 0.0; ///< The hit rate of the model.
    T_pred model; ///< The SVM model.

    int numSequenceElements = 0; ///< Number of sequence elements.
    int numClasses = 0; ///< Number of output classes.

    bool predictOnNonValidInput; ///< Flag indicating whether to predict on non-valid inputs.

    /**
     * @brief Destructor for the SVM class, performs cleanup.
     */
    ~SVM() {
        clean();
    }

    /**
     * @brief Cleans up the data and resets the internal state.
     *
     * This method clears the input data, output data, and masks used for simulation.
     */
    void clean() {
        this->inputData = vector<vector<float>>();
        this->outputData = vector<char>();
        this->predictableInputsMask = vector<char>();
        this->inputBufferMissesMask = vector<char>();
        this->dictionaryMissesMask = vector<char>();
    }

    /**
     * @brief Constructor for the SVM class with dataset input and sequence parameters.
     *
     * This constructor initializes the SVM with a dataset and the number of sequence elements and classes.
     *
     * @param classesDataset The dataset containing input-output pairs for training.
     * @param numSequenceElements The number of sequence elements.
     * @param numClasses The number of output classes.
     * @param predictOnNonValidInput Flag indicating whether to predict on non-valid input.
     */
    SVM(BuffersDataset<T_input> classesDataset, int numSequenceElements, int numClasses, bool predictOnNonValidInput) {
        static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Class is not sub-type of MultiSVMClassifier");

        this->numSequenceElements = numSequenceElements;
        this->numClasses = numClasses;
        this->predictOnNonValidInput = predictOnNonValidInput;
        if (predictOnNonValidInput) numInputClasses++;

        importData(classesDataset);
        initializeModel();
    }

    /**
     * @brief Constructor for the SVM class with sequence parameters.
     *
     * This constructor initializes the SVM with the number of sequence elements and classes.
     *
     * @param numSequenceElements The number of sequence elements.
     * @param numClasses The number of output classes.
     * @param predictOnNonValidInput Flag indicating whether to predict on non-valid input.
     */
    SVM(int numSequenceElements, int numClasses, bool predictOnNonValidInput) {
        static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Class is not sub-type of MultiSVMClassifier");

        this->numSequenceElements = numSequenceElements;
        this->numClasses = numClasses;
        this->predictOnNonValidInput = predictOnNonValidInput;
        if (predictOnNonValidInput) numInputClasses++;

        initializeModel();
    }

    /**
     * @brief Default constructor for the SVM class.
     *
     * This constructor initializes the SVM with default values for sequence elements and classes.
     */
    SVM() {
        static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Class is not sub-type of MultiSVMClassifier");

        this->numSequenceElements = 0;
        this->numClasses = 0;
        this->predictOnNonValidInput = true;
    }

    /**
     * @brief Imports input and output data into the SVM model from a dataset.
     *
     * This method processes the given dataset, normalizing the input data and adding it to the model.
     *
     * @param data The access dataset for the input data.
     * @param classesDataset The dataset containing input-output pairs for training.
     */
    void importData(AccessesDataset<L64bu, L64bu>& data, BuffersDataset<T_input>& classesDataset) {
        importData(classesDataset);
    }

    /**
     * @brief Imports input and output data into the SVM model from a dataset.
     *
     * This method processes the given dataset, normalizing the input data and adding it to the model.
     *
     * @param classesDataset The dataset containing input-output pairs for training.
     */
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

    /**
     * @brief Initializes the SVM model with hyperparameters.
     *
     * This method sets the model's hyperparameters (e.g., learning rate, regularization parameter) and initializes the classifier.
     */
    void initializeModel() {
        double c = 1.0;
        double learningRate = 0.7;
        model = T_pred(this->numSequenceElements, this->numClasses, c, 1, learningRate);
    }

    /**
     * @brief Fits the SVM model with the provided input and output.
     *
     * This method trains the model on a single input-output pair.
     *
     * @param input The input data to train the model with.
     * @param output The output label to train the model with.
     */
    void fit(vector<float> input, int output) {
        auto in = vector<vector<double>>{ vector<double>(input.begin(), input.end()) };
        auto out = vector<int>{ output };
        this->model.fit(in, out);
    }

    /**
     * @brief Predicts the output for a given input using the trained SVM model.
     *
     * This method makes a prediction for a single input data point.
     *
     * @param input The input data to predict the output for.
     * @return The predicted output.
     */
    int predict(vector<float> input) {
        auto in = vector<vector<double>>{ vector<double>(input.begin(), input.end()) };
        return this->model.predict(in)[0];
    }

    /**
     * @brief Simulates the SVM model on the dataset and calculates performance metrics.
     *
     * This method runs the SVM model on the input data, comparing the predicted outputs with the actual ones,
     * and calculating various performance metrics such as hit rate and miss rates.
     *
     * @param initialize Flag to specify whether to initialize the model before running the simulation.
     * @return A shared pointer to the prediction results and costs.
     */
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
				std::cout << "Hit rate: " << (double)numHits / (i + 1) << " ; " << ((double)i) / inputData.size() << std::endl;
			}

		}

		hitRate = ((double)numHits) / inputData.size();

		resultsAndCosts.hitRate = hitRate;
		resultsAndCosts.dictionaryMissRate = numDictionaryMisses / inputData.size();
		resultsAndCosts.cacheMissRate = numCacheMisses / inputData.size();
		resultsAndCosts.modelMemoryCost = getModelMemoryCosts();
		return shared_ptr<PredictResultsAndCosts>((PredictResultsAndCosts*) new BuffersSVMPredictResultsAndCosts(resultsAndCosts));
	}
    /**
     * @brief Calculates the memory cost of the SVM model.
     *
     * This method calculates the total memory cost required by the model, based on the number of SVM classifiers
     * and the number of features for each classifier. It returns the total memory cost in bytes.
     *
     * @return The total memory cost in bytes.
     */
    double getModelMemoryCosts() {
        int numSVMs = this->model.SVMsTable.size(); ///< Number of SVM classifiers in the model.
        int numElements = this->model.numFeatures + 1; ///< Number of elements per SVM (including the bias term).

        return numElements * sizeof(float) * numSVMs;
        // For now, we return the total number of bytes required for storing the model's weights and parameters.
    }

};


