#pragma once
#include <vector>
#include <time.h>
#include <math.h>

using namespace std;


long minWeight = +0L, maxWeight = -0L;

template<typename T>
string vectorToString(vector<T> v);

/**
 * Base, parent class which implements a standard SVM classifier given by a third-party.
*/
class SVMClassifier {
protected:
	vector<double> w;
	double b;
public:
	double c;
	unsigned int epochs;
	unsigned int seed;


	SVMClassifier() {}

	SVMClassifier(double c, unsigned int epochs, unsigned int seed) {
		this->c = c;
		this->epochs = epochs;
		this->seed = seed;
	}

	void setWeights(vector<double> w, double b) {
		this->w = w;
		this->b = b;
	}

	void initWeights(int numFeatures) {
		w.resize(numFeatures);
		b = 0;
	}


	/**
	 * (Not used)
	*/
	void fit(vector<vector<double>>& data, vector<int>& label) {
		srand(seed);

		if (w.size() == 0)
			w.resize(data[0].size());

		for (unsigned int t = 1; t <= epochs; t++) {

			unsigned int idx = rand() % data.size();

			if (label[idx] != 0) {
				double nt = epochs == 1 ? 1 : 1 / (c * t);

				vector<double> xi = data[idx];

				vector<double> next_w(xi.size(), 0);

				double dot_product = 0;


				for (unsigned int i = 0; i < xi.size(); i++) {
					dot_product += w[i] * xi[i];
				}

				if (dot_product * label[idx] < 1) {
					for (unsigned int k = 0; k < xi.size(); k++) {
						next_w[k] = w[k] - nt * c * w[k] + nt * label[idx] * xi[k];
					}
				}

				else //  if (dot_product * label[idx] > 0) {
					for (unsigned int k = 0; k < xi.size(); k++) {
						next_w[k] = w[k]; // - nt * c * w[k];
					}
				//}

				w = next_w;
			}


		}

		// for(unsigned int i = 0; i < w.size(); i++) {
		//     cout << "w" << i << " = " << w[i] << endl;
		// }

		// cout << endl;
	}

	/**
	 * Prediction of a given batch of feature vectors by checking their distance to the SVM's hyperplane.
	*/
	vector<int> predict(vector<vector<double>>& data) {
		vector<int> predicted_labels;

		for (unsigned int i = 0; i < data.size(); i++) {

			vector<double> xi = data[i];

			double dot_product = 0;

			for (unsigned int j = 0; j < xi.size(); j++) {
				dot_product += w[j] * xi[j];
			}

			if ((dot_product - b) >= 0) {
				predicted_labels.push_back(1);
			}

			else predicted_labels.push_back(-1);
		}

		return predicted_labels;
	}

	double  accuracy(vector<int>& label, vector<int>& pred_label);

	double accuracy(vector<vector<double>>& data, vector<int>& label);

};


/**
 * Function which returns the gradients of the parameters of a linear SVC via the sub-gradient descent method.
*/

inline void computeGradients(vector<double>& w, double b, vector<vector<double>>& x, vector<int>& y, double c,
	vector<double>& dw, double* pointer_db) {
	vector<double> resultingGradient_w = vector<double>(w.size(), 0);
	double resultingGradient_b = 0.0;

	for (int i = 0; i < x.size(); i++) {
		vector<double> partialGradient_w = vector<double>(w.size(), 0);
		double partialGradient_b = 0.0;
		double dot_product = 0;
		vector<double> xi = x[i];
		for (unsigned int j = 0; j < xi.size(); j++) {
			dot_product += w[j] * xi[j];
		}

		double distance = 1 - y[i] * (dot_product - b);
		if (distance <= 0) {
			for (unsigned int j = 0; j < xi.size(); j++) {
				partialGradient_w[j] += w[j] / 2 * (1 - c);
			}
			partialGradient_b += 0.0;
			// partialGradient_w = w;// vector<double>(w.size(), 0); // = w;

		}
		else {

			for (unsigned int j = 0; j < xi.size(); j++) {
				partialGradient_w[j] += (w[j] / 2 * (1 - c)) - (c * y[i] * xi[j]);
			}
			partialGradient_b += c * y[i];
		}

		for (unsigned int j = 0; j < xi.size(); j++) {
			resultingGradient_w[j] += partialGradient_w[j] / x.size();
		}
		resultingGradient_b += partialGradient_b / x.size();
	}

	dw = resultingGradient_w;
	*pointer_db = resultingGradient_b;
}


/**
 * Class which implements a SVM fitted by the sub-gradient descent method.
*/
class SVMSGDClassifier : public SVMClassifier {
public:
	double learningRate;
	SVMSGDClassifier() {}
	SVMSGDClassifier(double c, unsigned int epochs, unsigned int seed, double learningRate) :
		SVMClassifier(c, epochs, seed)
	{
		this->learningRate = learningRate;
	}

	/**
	* Initialization of the model parameters. Note that neither the normal vector 'w' nor
	* the intercept 'b'are set to zero since such state is inconsistent.
	*/
	void initWeights(int numFeatures) {
		// w.resize(numFeatures + 1);
		w = vector<double>(numFeatures, 0.5);
		b = 0.5;
	}

	/**
	* Fitting function, based on the computation of the gradients and the adjustment of
	* parameters w and b in case of prediction miss controlled by the stablished lerarning rate.
	*/
	void fit(vector<vector<double>>& data, vector<int>& label, double learningRate = 0) {

		if (w.size() == 0)
			w.resize(data[0].size());

		double learningRate_ = learningRate <= 0 ? this->learningRate : learningRate;

		for (unsigned int idx = 0; idx < data.size(); idx++) {


			vector<double> xi = data[idx];
			auto xi_ = vector<vector<double>>{ xi };
			auto label_ = vector<int>{ label[idx] };

			auto distanceBefore = computeDistanceToPlane(xi_, false); // DEBUG

			vector<double> gradient_w = vector<double>();
			double gradient_b = 0.0;
			computeGradients(w, b, xi_, label_, c, gradient_w, &gradient_b);

			for (int j = 0; j < w.size(); j++) {
				w[j] = w[j] - (learningRate_ * gradient_w[j]);
				double w_ = w[j] / learningRate_;
				if (w_ > maxWeight) {
					maxWeight = (long)w_;
				}
				if (w_ < minWeight) {
					minWeight = (long)w_;
				}
			}
			b = b - (learningRate_ * gradient_b);
			double b_ = b / learningRate_;
			if (b_ > maxWeight) {
				maxWeight = (long)b_;
			}
			if (b_ < minWeight) {
				minWeight = (long)b_;
			}

			auto distanceAfter = computeDistanceToPlane(xi_, false); // DEBUG

		}

		// for(unsigned int i = 0; i < w.size(); i++) {
		//     cout << "w" << i << " = " << w[i] << endl;
		// }

		// cout << endl;
	}

	/**
	* Given an input batch, the absolute distance from each feature vector to the hyperplane is returned.
	*/
	vector<double> computeDistanceToPlane(vector<vector<double>>& x, bool absoluteDistance) {
		vector<double> res = vector<double>();
		for (int i = 0; i < x.size(); i++) {
			double dot_product = 0;
			vector<double> xi = x[i];
			for (unsigned int j = 0; j < xi.size(); j++) {
				dot_product += w[j] * xi[j];
			}

			double distance = absoluteDistance ? abs(dot_product - b) : dot_product - b;
			res.push_back(distance);
		}
		return res;
	}
};

enum class MultiSVMClassifierType { OneToOne, OneToAll };

/**
 * A standard parent class for defining SVMs models based on SGD and multiple hyperplanes.
*/
class MultiSVMClassifier : public SVMSGDClassifier {
private:
	vector<double> w;
	unsigned int seed;

public:
	int numClasses;

	int numFeatures;


	vector<SVMSGDClassifier> SVMsTable;

	MultiSVMClassifier() {
		this->numClasses = 0;
		this->c = 0;
		this->epochs = 0;
		this->numClasses = 0;
		this->seed = 0;
		this->learningRate = 0;

	}


	MultiSVMClassifier(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate) {
		// MultiSVMClassifierType type) {

		this->numFeatures = numFeatures;
		this->numClasses = numClasses;
		this->c = c;
		this->epochs = epochs;
		this->seed = rand();
		this->learningRate = learningRate;

	}
	//MultiSVMClassifierType type = MultiSVMClassifierType::OneToOne);

	void initWeights(int numFeatures);

	vector<int> predict(vector<vector<double>>& data);


	void fit(vector<vector<double>>& data, vector<int>& label, double learningRate = 0);


};

/**
 * Class which implements the classification of N classes using NxN SVMs: one per possible class pair.
 * Not used.
*/
class MultiSVMClassifierOneToOne : public MultiSVMClassifier {

public:

	MultiSVMClassifierOneToOne() {

	}

	MultiSVMClassifierOneToOne(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate)
		: MultiSVMClassifier(numFeatures, numClasses, c, epochs, learningRate) {

		for (int i = 0; i < numClasses; i++)
			for (int j = 0; j < numClasses; j++) {
				if (j >= i) {
					auto model = SVMSGDClassifier(c, epochs, rand(), learningRate);
					model.initWeights(this->numFeatures);
					this->SVMsTable.push_back(model);

				}
				else {
					auto model = SVMSGDClassifier();
					model.initWeights(this->numFeatures);
					this->SVMsTable.push_back(model);
				}

			}

	}

	vector<int> predict(vector<vector<double>>& data) {
		vector<int> predicted_labels;

		vector<vector<int>> predictedLabelsPerPredictor;
		// Each classifier gives its prediction for the given data:
		for (int k = 0; k < SVMsTable.size(); k++) {
			predictedLabelsPerPredictor.push_back(SVMsTable[k].predict(data));
		}

		for (int i = 0; i < data.size(); i++) {
			vector<int> predictionsHistogram = vector<int>(numClasses, 0);
			int maxPreds = -1;
			int bestPred = -1;

			// We iterate through the whole set of classifiers:
			for (int j = 0; j < this->numClasses; j++)
				for (int k = 0; k < this->numClasses; k++) {
					if (k > j) {
						int class0 = j, class1 = k;
						int winnerClass;
						int predictorIndex = ((j * numClasses) + k);
						if (predictedLabelsPerPredictor[predictorIndex][i] == -1) {
							winnerClass = class0;
						}
						else {
							winnerClass = class1;
						}

						// We keep track of the predictions with a histogram:
						predictionsHistogram[winnerClass]++;

						// The current class with maximum number of predictions in total
						// is selected as best predicted class:
						if (predictionsHistogram[winnerClass] > maxPreds) {
							maxPreds = predictionsHistogram[winnerClass];
							bestPred = winnerClass;
						}


					}
				}

			// We append the best predicted class as predicted label:
			predicted_labels.push_back(bestPred);


		}

		return predicted_labels;
	}

	void fit(vector<vector<double>>& data, vector<int>& label, double learningRate = 0) {

		// For each classifier, we build the real-label vector for the entirety of given data:

		for (int j = 0; j < this->numClasses; j++)
			for (int k = 0; k < this->numClasses; k++) {
				if (k > j) {
					int class0 = j, class1 = k;
					int predictorIndex = ((j * numClasses) + k);
					vector<int> realLabels = vector<int>();
					if (label.size() > 1) {
						// std::transform(label.begin(), label.end(), realLabels.begin(),
						// 	[class0, class1](int c) {
						// 		int res = 0;
						// 		if (c == class0) res = -1;
						// 		else if(c == class1) res = +1;
						// 		return res;
						// 	});
						int res;
						for (int label_idx; label_idx < label.size(); label_idx++) {
							if (label[label_idx] == class0) res = -1;
							else if (label[label_idx] == class1) res = +1;
							realLabels.push_back(res);
						}
					}
					else {
						int res = 0;
						if (label[0] == class0) res = -1;
						else if (label[0] == class1) res = +1;
						realLabels = vector<int>{ res };
					}



					// Then, we fit each classifier with the resulting real-label vector:
					SVMsTable[predictorIndex].fit(data, realLabels, learningRate);
				}
			}

	}

};

/**
 * Class which implements the classification of N classes using N SVMs: one per class.
*/
class MultiSVMClassifierOneToAll : public MultiSVMClassifier {

public:

	MultiSVMClassifierOneToAll() {}

	MultiSVMClassifierOneToAll(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate) :
		MultiSVMClassifier(numFeatures, numClasses, c, epochs, learningRate) {

		for (int i = 0; i < numClasses; i++) {
			auto model = SVMSGDClassifier(c, epochs, rand(), learningRate);
			model.initWeights(this->numFeatures);
			this->SVMsTable.push_back(model);
		}

	}
	/**
	 * Parallel prediction on the SVMs, for which the predicted class is chosen as the one having the
	 * closest SVM. A whole batch is given as input.
	*/
	vector<int> predict(vector<vector<double>>& data) {
		vector<int> predicted_labels;

		vector<vector<int>> predictedLabelsPerPredictor;
		vector<vector<double>> distancesPerPredictor;
		vector<vector<double>> distancesPerPredictor_;
		// Each classifier gives its prediction for the given data:
		for (int k = 0; k < SVMsTable.size(); k++) {
			distancesPerPredictor_.push_back(SVMsTable[k].computeDistanceToPlane(data, true));
			distancesPerPredictor.push_back(SVMsTable[k].computeDistanceToPlane(data, false));
			predictedLabelsPerPredictor.push_back(SVMsTable[k].predict(data));
		}

		// For each data sample, we append as label the first class which has been predicted against 
		// the rest:
		for (int i = 0; i < data.size(); i++) {

			int bestPrediction = 0;
			double bestDistance = 0;

			for (int k = 0; k < SVMsTable.size(); k++) {
				// if (predictedLabelsPerPredictor[k][i] == -1) {
				if ((bestDistance == 0) || (distancesPerPredictor[k][i] < bestDistance)) {
					bestPrediction = k;
					bestDistance = distancesPerPredictor[k][i];
				}
				// }
				if (false)
					cout << "Plane " << to_string(k) << " with distance " << to_string(distancesPerPredictor[k][i]) << "\n";

			}
			if (false)
				cout << "Best prediction: " << to_string(bestPrediction) << "\n";

			predicted_labels.push_back(bestPrediction);

		}


		return predicted_labels;
	}
	/**
	 * Fitting of the SVMs given a batch and the target classes.
	*/
	void fit(vector<vector<double>>& data, vector<int>& label, double learningRate = 0) {

		// For each classifier, we build the real-label vector fpr the entirety of given data:
		for (int k = 0; k < SVMsTable.size(); k++) {
			vector<int> realLabels = vector<int>();
			if (label.size() > 1) {
				// std::transform(label.begin(), label.end(), realLabels.begin(),
				// 	[k](int c) {return k == c ? 0 : +1; });
				for (int label_idx = 0; label_idx < label.size(); label_idx++) {
					int res = k == label[label_idx] ? -1 : +1;
					realLabels.push_back(res);
				}
			}
			else realLabels = vector<int>{ k == label[0] ? -1 : +1 };

			// Then, we fit each classifier with the resulting real-label vector:
			SVMsTable[k].fit(data, realLabels, learningRate);
		}

	}

};

