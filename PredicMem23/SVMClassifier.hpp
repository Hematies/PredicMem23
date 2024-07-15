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

#ifndef SVM_CLASS_H
#define SVM_CLASS_H

#include <vector>
#include <time.h>

using namespace std;

class SVMClassifier {
protected:
	vector<double> w;
	double b;
public:
  double c;
  unsigned int epochs;
  unsigned int seed;
  

  SVMClassifier();

  SVMClassifier(double c, unsigned int epochs, unsigned int seed);

  void setWeights(vector<double> w, double b);

  void initWeights(int numFeatures);

  void fit(vector<vector<double>> & data, vector<int> & label);

  vector<int> predict(vector<vector<double>> & data);

  double  accuracy(vector<int> & label, vector<int> & pred_label);

  double accuracy(vector<vector<double>>& data, vector<int>& label);

};

class SVMSGDClassifier : public SVMClassifier {
public:
	double learningRate;
	SVMSGDClassifier();
	SVMSGDClassifier(double c, unsigned int epochs, unsigned int seed, double learningRate);
	void initWeights(int numFeatures);
	void fit(vector<vector<double>>& data, vector<int>& label);
	vector<double> computeDistanceToPlane(vector<vector<double>>& data);
};

enum class MultiSVMClassifierType { OneToOne, OneToAll };

class MultiSVMClassifier : public SVMSGDClassifier{
private:
	vector<double> w;
	unsigned int seed;

public:
	int numClasses;

	int numFeatures;


	vector<SVMSGDClassifier> SVMsTable;

	MultiSVMClassifier();
	
	MultiSVMClassifier(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate);
		//MultiSVMClassifierType type = MultiSVMClassifierType::OneToOne);

	void initWeights(int numFeatures);

	vector<int> predict(vector<vector<double>>& data);

	void fit(vector<vector<double>>& data, vector<int>& label);


};

class MultiSVMClassifierOneToOne : public MultiSVMClassifier {

public:

	MultiSVMClassifierOneToOne();
	
	MultiSVMClassifierOneToOne(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate);

	vector<int> predict(vector<vector<double>>& data);

	void fit(vector<vector<double>>& data, vector<int>& label);

};

class MultiSVMClassifierOneToAll : public MultiSVMClassifier {

public:

	MultiSVMClassifierOneToAll();

	MultiSVMClassifierOneToAll(int numFeatures, int numClasses, double c, unsigned int epochs, double learningRate);

	vector<int> predict(vector<vector<double>>& data);

	void fit(vector<vector<double>>& data, vector<int>& label);

};

#endif