#pragma once
#include "Global.h"

//template<typename T_access, typename T_delta, typename T_class>
template<typename T_access, typename T_class>
class PredictorModel {
public:
	virtual shared_ptr<PredictResultsAndCosts> simular(bool inicializar = true) = 0;
	virtual void importarDatos(AccessesDataset<T_access, T_access>& datos, BuffersDataset<T_class>& datasetClases) = 0;
	virtual void clean() = 0;
};