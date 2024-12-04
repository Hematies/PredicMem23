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

CacheParametersDomain decodeCacheParametersDomain(TiXmlElement* element) {
	auto res = CacheParametersDomain();
	for (TiXmlElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
		string childName = child->Value();
		if (childName == "numIndexBits") {
			res.numIndexBits.push_back(std::stoi(child->GetText()));
		}
		else if (childName == "numWays") {
			res.numWays.push_back(std::stoi(child->GetText()));
		}
		else if (childName == "numSequenceAccesses") {
			res.numSequenceAccesses.push_back(std::stoi(child->GetText()));
		}
		else if (childName == "saveHistoryAndClassIfNotValid") {
			res.saveHistoryAndClassIfNotValid.push_back((bool)std::stoi(child->GetText()));
		}
	}
	return res;
}


DictionaryParametersDomain decodeDictionaryParametersDomain(TiXmlElement* element) {
	auto res = DictionaryParametersDomain();
	for (TiXmlElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
		string childName = child->Value();
		if (childName == "numClasses") {
			res.numClasses.push_back(std::stoi(child->GetText()));
		}
		else if (childName == "maxConfidence") {
			res.maxConfidence.push_back(std::stoi(child->GetText()));
		}
		else if (childName == "numConfidenceJumps") {
			res.numConfidenceJumps.push_back(std::stoi(child->GetText()));
		}
		else if (childName == "saveHistoryAndClassIfNotValid") {
			res.saveHistoryAndClassIfNotValid.push_back((bool)std::stoi(child->GetText()));
		}
	}
	return res;
}

PredictorParametersDomain decodePredictorParametersDomain(TiXmlElement* element) {
	auto res = PredictorParametersDomain();

	for (TiXmlElement* child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
		string childName = child->Value();
		if (childName == "PredictorType") {
			res.types.push_back(stringToPredictorTable[child->GetText()]);
		}
		else if (childName == "cacheParams") {
			res.cacheParams = decodeCacheParametersDomain(child);
		}
		else if (childName == "additionalCacheParams") {
			res.additionalCacheParams = decodeCacheParametersDomain(child);
		}
		else if (childName == "dictParams") {
			res.dictParams = decodeDictionaryParametersDomain(child);
		}
	}
	return res;
}



TraceInfo decodeTraceInfo(TiXmlElement* element) {
	std::string name; ///< Trace name.
	std::string filename; ///< Trace filename.
	unsigned long numAccesses = 0L; ///< Number of accesses.

	for (TiXmlAttribute* attribute = element->FirstAttribute(); attribute != NULL; attribute = attribute->Next()) {
		string attributeName = attribute->Name();
		if (attributeName == "name") name = attribute->Value();
		else if (attributeName == "filename") filename = attribute->Value();
		else if (attributeName == "numAccesses") numAccesses = std::stoul(attribute->Value());
	}

	auto res = TraceInfo();
	res.filename = filename;
	res.name = name;
	res.numAccesses = numAccesses;

	return res;
}
