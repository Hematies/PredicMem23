#include "PredictorSVM.h"


template class PredictorSVM<MultiSVMClassifierOneToAll, int>;

template<typename T_pred, typename T_entrada>
PredictorSVM<T_pred, T_entrada>::~PredictorSVM() {
	clean();
}

template<typename T_pred, typename T_entrada>
void PredictorSVM<T_pred, T_entrada>::clean() {
	/*
	datosEntrada.clear();
	datosSalida.clear();
	mascaraEntradasPredecibles.clear();
	mascaraErroresCache.clear();
	mascaraErroresDiccionario.clear();
	*/
	this->datosEntrada = vector<vector<float>>();
	this->datosSalida = vector<char>();
	this->mascaraEntradasPredecibles = vector<char>();
	this->mascaraErroresCache = vector<char>();
	this->mascaraErroresDiccionario = vector<char>();
}


template<typename T_pred, typename T_entrada>
void PredictorSVM<T_pred, T_entrada>::importarDatos(AccessesDataset<L64bu, L64bu>& datos, BuffersDataset<T_entrada>& datasetClases) {
	importarDatos(datasetClases);
}

template<typename T_pred, typename T_entrada>
void PredictorSVM<T_pred, T_entrada>::importarDatos(BuffersDataset<T_entrada>& datasetClases) {
	for (int i = 0; i < datasetClases.inputAccesses.size(); i++) {
		vector<float> entrada = vector<float>();
		char salida = -1;
		char esEntradaValida = false;

		for (int j = 0; j < datasetClases.inputAccesses[i].size(); j++) {
			entrada.push_back(((float)datasetClases.inputAccesses[i][j]) / numClasesEntrada + 1.0);
		}

		salida = datasetClases.outputAccesses[i];
		esEntradaValida = datasetClases.isValid[i];

		this->datosEntrada.push_back(entrada);
		this->datosSalida.push_back(salida);
		this->mascaraEntradasPredecibles.push_back(esEntradaValida);
		this->mascaraErroresCache.push_back(datasetClases.isCacheMiss[i]);
		this->mascaraErroresDiccionario.push_back(datasetClases.isDictionaryMiss[i]);
	}
}

template<typename T_pred, typename T_entrada>
void PredictorSVM<T_pred, T_entrada>::inicializarModelo() {
	// double c = 0.5;
	double c = 1.0;
	double learningRate = 0.7;
	// double c = 0.8;
	modelo = T_pred(this->numElemSecuencia, this->numClases, c, 1, learningRate);
	// ajustarPredictor(std::vector<float>(datosEntrada[0].size(), 0.0), 0);
}

template<typename T_pred, typename T_entrada>
void PredictorSVM<T_pred, T_entrada>::ajustarPredictor(vector<float> entrada, int salida) {
	auto in = vector<vector<double>>{ vector<double>(entrada.begin(), entrada.end()) };
	auto out = vector<int>{ salida };
	this->modelo.fit(in, out);
}

template<typename T_pred, typename T_entrada>
int PredictorSVM<T_pred, T_entrada>::predecir(vector<float> entrada) {
	auto in = vector<vector<double>>{ vector<double>(entrada.begin(), entrada.end()) };
	return this->modelo.predict(in)[0];
}

template<typename T_pred, typename T_entrada>
shared_ptr<PredictResultsAndCosts> PredictorSVM<T_pred, T_entrada>::simular(bool inicializar) {

	BuffersSVMPredictResultsAndCosts resultsAndCosts = BuffersSVMPredictResultsAndCosts();
	double numDictionaryMisses = 0.0;
	double numCacheMisses = 0.0;


	if (inicializar) {
		this->inicializarModelo();

	}

	numAciertos = 0;
	tasaExito = 0.0;

	for (int i = 0; i < datosEntrada.size(); i++) {
		vector<float> entrada = vector<float>(datosEntrada[i].begin(), datosEntrada[i].end());
		int salida = datosSalida[i];
		auto esEntradaPredecible = mascaraEntradasPredecibles[i];
		auto haHabidoErrorCache = mascaraErroresCache[i];
		auto haHabidoErrorDiccionario = mascaraErroresDiccionario[i];

		int salidaPredicha = -1;
		if (esEntradaPredecible)
			salidaPredicha = predecir(entrada);

		bool haHabidoFalloPrediccion = (salida != salidaPredicha);

		// Si ha habido un fallo, entrenamos con la muestra de entrada y salida:
		// bool hayQueAjustar = (haHabidoFalloPrediccion && esEntradaPredecible) || (haHabidoErrorDiccionario && !esEntradaPredecible);
		bool hayQueAjustar = !haHabidoErrorCache && (haHabidoFalloPrediccion || haHabidoErrorDiccionario);
		if (hayQueAjustar) {
			ajustarPredictor(entrada, salida);
		}

		bool hayHit = esEntradaPredecible && !haHabidoErrorDiccionario && !haHabidoErrorCache && !haHabidoFalloPrediccion;
		if (hayHit)
			numAciertos++;

		if (haHabidoErrorDiccionario) numDictionaryMisses++;
		if (haHabidoErrorCache) numCacheMisses++;


		if (i % numPartesMostrar == 0) {
			// 
			// if (!esEntradaPredecible){
			string in = "";
			for (auto e : entrada)
				in += to_string((e - 1.0) * numClasesEntrada) + ", ";
			std::cout << in << " -> " << salida << " vs " << salidaPredicha << std::endl;
			std::cout << "Tasa de éxito: " << (double)numAciertos / (i + 1) << " ; " << ((double)i) / datosEntrada.size() << std::endl;
		}

	}

	tasaExito = ((double)numAciertos) / datosEntrada.size();

	resultsAndCosts.hitRate = tasaExito;
	resultsAndCosts.dictionaryMissRate = numDictionaryMisses / datosEntrada.size();
	resultsAndCosts.cacheMissRate = numCacheMisses / datosEntrada.size();
	resultsAndCosts.modelMemoryCost = getModelMemoryCosts();
	return shared_ptr<PredictResultsAndCosts>((PredictResultsAndCosts*) new BuffersSVMPredictResultsAndCosts(resultsAndCosts));
}


template<typename T_pred, typename T_entrada>
double PredictorSVM<T_pred, T_entrada>::getModelMemoryCosts() {
	int numSVMs = this->modelo.SVMsTable.size();
	int numElements = this->modelo.numFeatures + 1;

	return numElements * sizeof(float) * numSVMs;
	// For now, we return the total number bytes of the elements (weights) of the model.
}
