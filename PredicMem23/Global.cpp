#include "Global.h"


vector<PredictorParameters> decomposeCacheParameters(vector<PredictorParameters>& base, CacheParametersDomain& domain, vector<string> params,
	bool isAdditionalCache) {
	string currentParam = params[0];
	auto res = vector<PredictorParameters>();
	if (currentParam == "numIndexBits") {
		for (auto& value : domain.numIndexBits) {
			for (PredictorParameters predictorParams : base) {
				if(!isAdditionalCache)
					predictorParams.cacheParams.numIndexBits = value;
				else
					predictorParams.additionalCacheParams.numIndexBits = value;
				res.push_back(predictorParams);
			}
		}
	}
	else if (currentParam == "numWays") {
		for (auto& value : domain.numWays) {
			for (PredictorParameters predictorParams : base) {
				if (!isAdditionalCache)
					predictorParams.cacheParams.numWays = value;
				else
					predictorParams.additionalCacheParams.numWays = value;
				res.push_back(predictorParams);
			}
		}
	}
	else if (currentParam == "numSequenceAccesses") {
		for (auto& value : domain.numSequenceAccesses) {
			for (PredictorParameters predictorParams : base) {
				if (!isAdditionalCache)
					predictorParams.cacheParams.numSequenceAccesses = value;
				else
					predictorParams.additionalCacheParams.numSequenceAccesses = value;
				res.push_back(predictorParams);
			}
		}
	}
	else if (currentParam == "saveHistoryAndClassIfNotValid") {
		for (auto value : domain.saveHistoryAndClassIfNotValid) {
			for (PredictorParameters predictorParams : base) {
				if (!isAdditionalCache)
					predictorParams.cacheParams.saveHistoryAndClassIfNotValid = value;
				else
					predictorParams.additionalCacheParams.saveHistoryAndClassIfNotValid = value;
				res.push_back(predictorParams);
			}
		}
	}
	else throw - 1;

	if (params.size() > 1) {
		params.erase(params.begin());
		res = decomposeCacheParameters(res, domain, params, isAdditionalCache);
	}

	return res;
}



vector<PredictorParameters> decomposeCacheParametersBegin(vector<PredictorParameters>& base, CacheParametersDomain& domain,
	bool isAdditionalCache) {
	vector<string> params = vector<string>{ "numIndexBits", "numWays", "numSequenceAccesses", "saveHistoryAndClassIfNotValid" };
	return decomposeCacheParameters(base, domain, params, isAdditionalCache);
}


vector<PredictorParameters> decomposeDictionaryParameters(vector<PredictorParameters>& base, DictionaryParametersDomain& domain, vector<string> params) {
	string currentParam = params[0];
	auto res = vector<PredictorParameters>();
	if (currentParam == "numClasses") {
		for (auto& value : domain.numClasses) {
			for (PredictorParameters predictorParams : base) {
				predictorParams.dictParams.numClasses = value;
				res.push_back(predictorParams);
			}
		}
	}
	else if (currentParam == "maxConfidence") {
		for (auto& value : domain.maxConfidence) {
			for (PredictorParameters predictorParams : base) {
				predictorParams.dictParams.maxConfidence = value;
				res.push_back(predictorParams);
			}
		}
	}
	else if (currentParam == "numConfidenceJumps") {
		for (auto& value : domain.numConfidenceJumps) {
			for (PredictorParameters predictorParams : base) {
				predictorParams.dictParams.numConfidenceJumps = value;
				res.push_back(predictorParams);
			}
		}
	}
	else if (currentParam == "saveHistoryAndClassIfNotValid") {
		for (auto value : domain.saveHistoryAndClassIfNotValid) {
			for (PredictorParameters predictorParams : base) {
				predictorParams.dictParams.saveHistoryAndClassIfNotValid = value;
				res.push_back(predictorParams);
			}
		}
	}
	else throw - 1;

	if (params.size() > 1) {
		params.erase(params.begin());
		res = decomposeDictionaryParameters(res, domain, params);
	}

	return res;
}


vector<PredictorParameters> decomposeDictionaryParametersBegin(vector<PredictorParameters>& base, DictionaryParametersDomain& domain) {
	vector<string> params = vector<string>{ "numClasses", "maxConfidence", "numConfidenceJumps", "saveHistoryAndClassIfNotValid" };
	return decomposeDictionaryParameters(base, domain, params);
}

vector<PredictorParameters> decomposePredictorParametersDomain(PredictorParametersDomain paramsDomain) {
	vector<PredictorParameters> base = vector<PredictorParameters>();
	for (auto& type : paramsDomain.types) {
		PredictorParameters params;
		params.type = type;
		base.push_back(params);
	}

	base = decomposeCacheParametersBegin(base, paramsDomain.cacheParams, false);
	base = decomposeCacheParametersBegin(base, paramsDomain.additionalCacheParams, true);
	base = decomposeDictionaryParametersBegin(base, paramsDomain.dictParams);

	return base;
}
