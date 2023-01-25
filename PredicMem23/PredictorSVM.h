#pragma once
#include <string>
#include <iostream>
#include "SVMClassifier.hpp"
#include "BuffersSimulator.h"

using namespace std;



template<typename T_pred, typename T_entrada>
class PredictorSVM
{

private:
	int numPartesMostrar = 1000;
public:
	vector<vector<float>> datosEntrada = vector<vector<float>>();
	vector<char> datosSalida = vector<char>();
	vector<char> mascaraErroresBufferes = vector<char>();
	long numAciertos = 0;
	int numMuestrasLote = 1;
	int numRepeticiones = 1;
	double tasaExito = 0.0;
	T_pred modelo;

	int numElemSecuencia = 0;
	int numClases = 0;


	PredictorSVM(BuffersDataset<T_entrada> datasetClases, int numElemSecuencia, int numClases) {

		static_assert(std::is_base_of<MultiSVMClassifier, T_pred>::value, "Clase no es subtipo de MultiSVMClassifier");

		this->numElemSecuencia = numElemSecuencia;
		this->numClases = numClases;
		importarDatos(datasetClases);
		inicializarModelo();
	}

	/*
	void importarDatos(string nombreFichero) {
		Mat datos = leerMat(nombreFichero);

		for (int i = 0; i < datos.rows; i++) {
			vector<T_CLASE> entrada = vector<T_CLASE>();
			char salida = -1;
			char haHabidoErrorVocabulario = false;

			for (int j = 0; j < datos.cols - 2; j++) {
				entrada.push_back((T_CLASE)datos.at<uchar>(i, j) / numElemSecuencia + 1.0);

			}
			salida = datos.at<uchar>(i, datos.cols - 2);
			haHabidoErrorVocabulario = datos.at<uchar>(i, datos.cols - 1);

			this->datosEntrada.push_back(entrada);
			this->datosSalida.push_back(salida);
			this->mascaraErroresBufferes.push_back(haHabidoErrorVocabulario);
		}
	}
	*/


	void importarDatos(BuffersDataset<T_entrada> datasetClases) {
		for (int i = 0; i < datasetClases.inputAccesses.size(); i++) {
			vector<float> entrada = vector<float>();
			char salida = -1;
			char haHabidoErrorBufferes = false;

			for (int j = 0; j < datasetClases.inputAccesses[i].size(); j++) {
				entrada.push_back(datasetClases.inputAccesses[i][j] / numElemSecuencia + 1.0);
			}

			salida = datasetClases.outputAccesses[i];
			haHabidoErrorBufferes = !datasetClases.isValid[i];

			this->datosEntrada.push_back(entrada);
			this->datosSalida.push_back(salida);
			this->mascaraErroresBufferes.push_back(haHabidoErrorBufferes);
		}
	}

	void inicializarModelo() {
		// double c = 0.5;
		double c = 1.0;
		double learningRate = 0.5;
		// double c = 0.8;
		modelo = T_pred(this->numElemSecuencia, this->numClases, c, 1, learningRate);
		// ajustarPredictor(std::vector<float>(datosEntrada[0].size(), 0.0), 0);
	}

	void ajustarPredictor(vector<float> entrada, int salida) {
		auto in = vector<vector<double>>{ vector<double>(entrada.begin(), entrada.end()) };
		auto out = vector<int>{ salida };
		this->modelo.fit(in, out);
	}

	int predecir(vector<float> entrada) {
		auto in = vector<vector<double>>{ vector<double>(entrada.begin(), entrada.end()) };
		return this->modelo.predict(in)[0];
	}

	void simular(bool inicializar = true) {

		if (inicializar) {
			this->inicializarModelo();

		}

		numAciertos = 0;
		tasaExito = 0.0;

		for (int i = 0; i < datosEntrada.size(); i++) {
			vector<float> entrada = vector<float>(datosEntrada[i].begin(), datosEntrada[i].end());
			int salida = datosSalida[i];
			auto haHabidoErrorBufferes = mascaraErroresBufferes[i];

			int salidaPredicha = -1;
			if(!haHabidoErrorBufferes)
				salidaPredicha = predecir(entrada);

			bool haHabidoFalloPrediccion = (salida != salidaPredicha);
			bool haHabidoFallo = haHabidoFalloPrediccion || haHabidoErrorBufferes;

			// Si ha habido un fallo, entrenamos con la muestra de entrada y salida:
			if (haHabidoFalloPrediccion) {
				ajustarPredictor(entrada, salida);
			}
			else if(!haHabidoErrorBufferes) numAciertos++;

			// if (i % numPartesMostrar == 0) {
			// 
			if (true){
				string in = "";
				for (auto e : entrada)
					in += to_string((e - 1.0) * numElemSecuencia) + ", ";
				std::cout << in << " -> " << salida << " vs " << salidaPredicha << std::endl;
				std::cout << "Tasa de éxito: " << (double)numAciertos / (i + 1) << " ; " << ((double)i) / datosEntrada.size() << std::endl;
			}

		}

		tasaExito = ((double)numAciertos) / datosEntrada.size();
	}
};


