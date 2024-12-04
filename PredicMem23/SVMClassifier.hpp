/////////////////////////////////////////////////////////////////////////////////
/// @file SVMClassifier.h
/// @brief Definitions of classes for SVM-based classification models.
///
/// This file defines classes for SVM classification including basic SVMs, 
/// stochastic gradient descent SVMs, and multi-class SVM implementations.
///
/// @details
/// Classes included:
/// - SVMClassifier: Base class for SVM.
/// - SVMSGDClassifier: Stochastic Gradient Descent-based SVM.
/// - MultiSVMClassifier: Base class for multi-class SVMs.
/// - MultiSVMClassifierOneToOne: One-vs-One multi-class SVM.
/// - MultiSVMClassifierOneToAll: One-vs-All multi-class SVM.
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
/////////////////////////////////////////////////////////////////////////////////

#ifndef SVM_CLASS_H
#define SVM_CLASS_H

#include <vector>
#include <time.h>

using namespace std;

/// @brief Base class for Support Vector Machine (SVM) classifiers.
class SVMClassifier {
protected:
    vector<double> w; ///< Weight vector.
    double b; ///< Bias term.

public:
    double c; ///< Regularization parameter.
    unsigned int epochs; ///< Number of training epochs.
    unsigned int seed; ///< Random seed for initialization.

    /**
     * @brief Default constructor.
     */
    SVMClassifier();

    /**
     * @brief Parameterized constructor.
     * @param c Regularization parameter.
     * @param epochs Number of training epochs.
     * @param seed Random seed for initialization.
     */
    SVMClassifier(double c, unsigned int epochs, unsigned int seed);

    /**
     * @brief Set the SVM weights and bias.
     * @param w Weight vector.
     * @param b Bias term.
     */
    void setWeights(vector<double> w, double b);

    /**
     * @brief Initialize weights with random values.
     * @param numFeatures Number of features in the data.
     */
    void initWeights(int numFeatures);

    /**
     * @brief Train the SVM on labeled data.
     * @param data Training data.
     * @param label Corresponding labels for the training data.
     */
    void fit(vector<vector<double>>& data, vector<int>& label);

    /**
     * @brief Predict labels for a given dataset.
     * @param data Data to classify.
     * @return Predicted labels.
     */
    vector<int> predict(vector<vector<double>>& data);

    /**
     * @brief Calculate classification accuracy.
     * @param label True labels.
     * @param pred_label Predicted labels.
     * @return Accuracy value.
     */
    double accuracy(vector<int>& label, vector<int>& pred_label);

    /**
     * @brief Calculate classification accuracy using the model's predictions.
     * @param data Data to classify.
     * @param label True labels.
     * @return Accuracy value.
     */
    double accuracy(vector<vector<double>>& data, vector<int>& label);
};

/// @brief SVM classifier trained using stochastic gradient descent (SGD).
class SVMSGDClassifier : public SVMClassifier {
public:
    double learningRate; ///< Learning rate for SGD.

    /**
     * @brief Default constructor.
     */
    SVMSGDClassifier();

    /**
     * @brief Parameterized constructor.
     * @param c Regularization parameter.
     * @param epochs Number of training epochs.
     * @param seed Random seed for initialization.
     * @param learningRate Learning rate for SGD.
     */
    SVMSGDClassifier(double c, unsigned int epochs, unsigned int seed, double learningRate);

    /**
     * @brief Initialize weights with random values.
     * @param numFeatures Number of features in the data.
     */
    void initWeights(int numFeatures);

    /**
     * @brief Train the SVM using stochastic gradient descent.
     * @param data Training data.
     * @param label Corresponding labels for the training data.
     */
    void fit(vector<vector<double>>& data, vector<int>& label);

    /**
     * @brief Compute distances from data points to the decision boundary.
     * @param data Data to classify.
     * @return Distances from the decision boundary.
     */
    vector<double> computeDistanceToPlane(vector<vector<double>>& data);
};

/// @brief Types of multi-class SVM classifiers.
enum class MultiSVMClassifierType { OneToOne, OneToAll };

/// @brief Base class for multi-class SVM classifiers.
class MultiSVMClassifier : public SVMSGDClassifier {
private:
    vector<double> w; ///< Weight vector for multi-class classification.
    unsigned int seed; ///< Random seed for initialization.

public:
    int numClasses; ///< Number of classes.
    int numFeatures; ///< Number of features.
    vector<SVMSGDClassifier> SVMsTable; ///< Collection of binary classifiers.

    /**
     * @brief Default constructor.
     */
    MultiSVMClassifier();

    /**
     * @brief Parameterized constructor.
     * @param numFeatures Number of features in the data.
     * @param numClasses Number of classes.
     * @param c Regularization parameter.
     * @param epochs Number of training epochs.
     * @param learningRate Learning rate for SGD.
     */
    MultiSVMClassifier(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate);

    /**
     * @brief Initialize weights with random values.
     * @param numFeatures Number of features in the data.
     */
    void initWeights(int numFeatures);

    /**
     * @brief Predict labels for a given dataset.
     * @param data Data to classify.
     * @return Predicted labels.
     */
    vector<int> predict(vector<vector<double>>& data);

    /**
     * @brief Train the multi-class SVM.
     * @param data Training data.
     * @param label Corresponding labels for the training data.
     */
    void fit(vector<vector<double>>& data, vector<int>& label);
};

/// @brief One-vs-One multi-class SVM classifier.
class MultiSVMClassifierOneToOne : public MultiSVMClassifier {
public:
    /**
     * @brief Default constructor.
     */
    MultiSVMClassifierOneToOne();

    /**
     * @brief Parameterized constructor.
     * @param numFeatures Number of features in the data.
     * @param numClasses Number of classes.
     * @param c Regularization parameter.
     * @param epochs Number of training epochs.
     * @param learningRate Learning rate for SGD.
     */
    MultiSVMClassifierOneToOne(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate);

    /**
     * @brief Predict labels for a given dataset.
     * @param data Data to classify.
     * @return Predicted labels.
     */
    vector<int> predict(vector<vector<double>>& data);

    /**
     * @brief Train the one-vs-one multi-class SVM.
     * @param data Training data.
     * @param label Corresponding labels for the training data.
     */
    void fit(vector<vector<double>>& data, vector<int>& label);
};

/// @brief One-vs-All multi-class SVM classifier.
class MultiSVMClassifierOneToAll : public MultiSVMClassifier {
public:
    /**
     * @brief Default constructor.
     */
    MultiSVMClassifierOneToAll();

    /**
     * @brief Parameterized constructor.
     * @param numFeatures Number of features in the data.
     * @param numClasses Number of classes.
     * @param c Regularization parameter.
     * @param epochs Number of training epochs.
     * @param learningRate Learning rate for SGD.
     */
    MultiSVMClassifierOneToAll(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate);

    /**
     * @brief Predict labels for a given dataset.
     * @param data Data to classify.
     * @return Predicted labels.
     */
    vector<int> predict(vector<vector<double>>& data);

    /**
     * @brief Train the one-vs-all multi-class SVM.
     * @param data Training data.
     * @param label Corresponding labels for the training data.
     */
    void fit(vector<vector<double>>& data, vector<int>& label);
};

#endif
