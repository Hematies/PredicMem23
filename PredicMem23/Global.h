/////////////////////////////////////////////////////////////////////////////////
/// @file Global.h
/// @brief Definition of structures and functions for the PredicMem23 project.
///
/// This file contains the definitions of various structures and functions 
/// used in the PredicMem23 project.
///
/// @details
/// The following structures and functions are included:
/// - ModelParameters
/// - CacheParameters
/// - CacheParametersDomain
/// - decodeCacheParametersDomain
/// - DictionaryParameters
/// - DictionaryParametersDomain
/// - decodeDictionaryParametersDomain
/// - PredictorModelType
/// - PredictorParameters
/// - PredictorParametersDomain
/// - decomposePredictorParametersDomain
/// - PredictResultsAndCosts
/// - BuffersSVMPredictResultsAndCosts
/// - DFCMPredictResultsAndCosts
/// - TraceInfo
/// - decodeTraceInfo
/// - AccessesDataset
/// - BuffersDataset
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
///
/// @section DESCRIPTION
/// This file declares the structures and functions for the PredicMem23 project.
/// It includes definitions for model parameters, cache parameters, dictionary parameters,
/// predictor parameters, and the decomposition of predictor parameters domains.
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include<stdio.h>
#include <string>
#include <map>
#include <vector>
#include <tinyxml.h>

using namespace std;
using std::vector;
using std::map;

#define L64bu unsigned long long
#define L64b long long

/**
 * @brief Structure to store model parameters.
 */
struct ModelParameters {

};

/**
 * @brief Structure to store cache parameters.
 */
struct CacheParameters {
	int numIndexBits; ///< Number of index bits.
	int numWays; ///< Number of ways.
	int numSequenceAccesses; ///< Number of sequence accesses.
	bool saveHistoryAndClassIfNotValid; ///< Flag to save history and class if not valid.
};


/**
 * @brief Structure to store domain of cache parameters.
 */
struct CacheParametersDomain {
	vector<int> numIndexBits; ///< Domain of number of index bits.
	vector<int> numWays; ///< Domain of number of ways.
	vector<int> numSequenceAccesses; ///< Domain of number of sequence accesses.
	vector<bool> saveHistoryAndClassIfNotValid; ///< Domain of save history and class if not valid flag.
};

/**
*  @brief Decodification function from XML to CacheParametersDomain.
* @param element XML element that contains the CacheParametersDomain
*/
CacheParametersDomain decodeCacheParametersDomain(TiXmlElement* element);


/**
 * @brief Structure to store dictionary parameters.
 */
struct DictionaryParameters {
	int numClasses; ///< Number of classes.
	int maxConfidence; ///< Maximum confidence.
	int numConfidenceJumps; ///< Number of confidence jumps.
	bool saveHistoryAndClassIfNotValid; ///< Flag to save history and class if not valid.
};

/**
 * @brief Structure to store domain of dictionary parameters.
 */
struct DictionaryParametersDomain {
	vector<int> numClasses; ///< Domain of number of classes.
	vector<int> maxConfidence; ///< Domain of maximum confidence.
	vector<int> numConfidenceJumps; ///< Domain of number of confidence jumps.
	vector<bool> saveHistoryAndClassIfNotValid; ///< Domain of save history and class if not valid flag.
};

/**
*  @brief Decodification function from XML to DictionaryParametersDomain.
* @param element XML element that contains the DictionaryParametersDomain
*/
DictionaryParametersDomain decodeDictionaryParametersDomain(TiXmlElement* element);

/**
 * @brief Enum class for predictor model types.
 */
enum class PredictorModelType { BufferSVM, DFCM };

/**
 * @brief Structure to store predictor parameters.
 */
struct PredictorParameters {
	PredictorModelType type; ///< Predictor model type.
	CacheParameters cacheParams; ///< Cache parameters.
	CacheParameters additionalCacheParams; ///< Additional cache parameters.
	DictionaryParameters dictParams; ///< Dictionary parameters.
};

/**
 * @brief Structure to store domain of predictor parameters.
 */
struct PredictorParametersDomain {
	vector<PredictorModelType> types; ///< Domain of predictor model types.
	CacheParametersDomain cacheParams; ///< Domain of cache parameters.
	CacheParametersDomain additionalCacheParams; ///< Domain of additional cache parameters.
	DictionaryParametersDomain dictParams; ///< Domain of dictionary parameters.
};

/**
*  @brief Decodification function from XML to PredictorParametersDomain.
* @param element XML element that contains the PredictorParametersDomain
*/
PredictorParametersDomain decodePredictorParametersDomain(TiXmlElement* element);

/**
 * @brief Decomposes a predictor parameters domain into a vector of predictor parameters.
 *
 * @param paramsDomain The predictor parameters domain to decompose.
 * @return A vector of predictor parameters.
 */
vector<PredictorParameters> decomposePredictorParametersDomain(PredictorParametersDomain paramsDomain);

/**
 * @brief Abstract class for prediction results and costs.
 */
class PredictResultsAndCosts {
public:
	/**
	 * @brief Get the hit rate.
	 * @return The hit rate.
	 */
	virtual double getHitRate() = 0;
	/**
	 * @brief Set the hit rate.
	 * @param hitRate The hit rate to set.
	 */
	virtual void setHitRate(double) = 0;
	/**
	 * @brief Get the total memory cost.
	 * @return The total memory cost.
	 */
	virtual double getTotalMemoryCost() = 0;
	/**
	* @brief Set the total memory cost.
	* @param totalMemoryCost The total memory cost to set.
	*/
	virtual void setTotalMemoryCost(double) = 0;
	/**
	 * @brief Get the results and costs as a map.
	 * @return A map of results and costs.
	 */
	virtual map<string, double> getResultsAndCosts() = 0;
};

/**
 * @brief Class for SVM prediction results and costs.
 */
class BuffersSVMPredictResultsAndCosts : PredictResultsAndCosts {
public:
	double hitRate = 0.0; ///< Hit rate.
	double cacheMissRate = 0.0; ///< Cache miss rate.
	double dictionaryMissRate = 0.0; ///< Dictionary miss rate.
	double cacheMemoryCost = 0.0; ///< Cache memory cost.
	double dictionaryMemoryCost = 0.0; ///< Dictionary memory cost.
	double modelMemoryCost = 0.0; ///< Model memory cost.
	double totalMemoryCost = 0.0; ///< Total memory cost.

	/**
	 * @brief Default constructor.
	 */
	BuffersSVMPredictResultsAndCosts() {}

	/**
	 * @brief Constructor with parameters.
	 * @param hitRate Hit rate.
	 * @param cacheMissRate Cache miss rate.
	 * @param dictionaryMissRate Dictionary miss rate.
	 * @param cacheMemoryCost Cache memory cost.
	 * @param dictionaryMemoryCost Dictionary memory cost.
	 * @param modelMemoryCost Model memory cost.
	 */
	BuffersSVMPredictResultsAndCosts(double hitRate, double cacheMissRate, double dictionaryMissRate,
		double cacheMemoryCost, double dictionaryMemoryCost, double modelMemoryCost){
		this->hitRate = hitRate;
		this->cacheMissRate = cacheMissRate;
		this->dictionaryMissRate = dictionaryMissRate;
		this->cacheMemoryCost = cacheMemoryCost;
		this->dictionaryMemoryCost = dictionaryMemoryCost;
		this->modelMemoryCost = modelMemoryCost;
		this->totalMemoryCost = cacheMemoryCost + dictionaryMemoryCost + modelMemoryCost;
	}

	/**
	 * @brief Get the hit rate.
	 * @return The hit rate.
	 */
	double getHitRate() { return hitRate; }
	/**
	* @brief Set the hit rate.
	* @param hitRate The hit rate to set.
	*/
	void setHitRate(double hitRate) { this->hitRate = hitRate; }
	/**
	 * @brief Get the total memory cost.
	 * @return The total memory cost.
	 */
	double getTotalMemoryCost() { return totalMemoryCost; }
	/**
	 * @brief Set the total memory cost.
	 * @param totalMemoryCost The total memory cost to set.
	 */
	void setTotalMemoryCost(double totalMemoryCost) { this->totalMemoryCost = totalMemoryCost; }

	/**
	 * @brief Get the results and costs as a map.
	 * @return A map of results and costs.
	 */
	map<string, double> getResultsAndCosts() {
		return {
			{"hitRate", hitRate},
			{"cacheMissRate", cacheMissRate},
			{"dictionaryMissRate", dictionaryMissRate},
			{"cacheMemoryCost", cacheMemoryCost},
			{"dictionaryMemoryCost", dictionaryMemoryCost},
			{"modelMemoryCost", modelMemoryCost},
			{"totalMemoryCost", totalMemoryCost},
		};
	}
};


/**
 * @brief Class for DFCM prediction results and costs.
 */
class DFCMPredictResultsAndCosts : PredictResultsAndCosts {
public:
	double hitRate = 0.0; ///< Hit rate.
	double firstTableMissRate = 0.0; ///< First table miss rate.
	double secondTableMissRate = 0.0; ///< Second table miss rate.
	double firstTableMemoryCost = 0.0; ///< First table memory cost.
	double secondTableMemoryCost = 0.0; ///< Second table memory cost.
	double totalMemoryCost = 0.0; ///< Total memory cost.

	/**
	 * @brief Default constructor.
	 */
	DFCMPredictResultsAndCosts() {}

	/**
	 * @brief Constructor with parameters.
	 * @param hitRate Hit rate.
	 * @param firstTableMissRate First table miss rate.
	 * @param secondTableMissRate Second table miss rate.
	 * @param firstTableMemoryCost First table memory cost.
	 * @param secondTableMemoryCost Second table memory cost.
	 */
	DFCMPredictResultsAndCosts(double hitRate, double firstTableMissRate, double secondTableMissRate,
		double firstTableMemoryCost, double secondTableMemoryCost) {
		this->hitRate = hitRate;
		this->firstTableMissRate = firstTableMissRate;
		this->secondTableMissRate = secondTableMissRate;
		this->firstTableMemoryCost = firstTableMemoryCost;
		this->secondTableMemoryCost = secondTableMemoryCost;
		this->totalMemoryCost = firstTableMemoryCost + secondTableMemoryCost;
	}

	/**
	* @brief Get the hit rate.
	* @return The hit rate.
	*/
	double getHitRate() { return hitRate; }
	/**
	 * @brief Set the hit rate.
	 * @param hitRate The hit rate to set.
	 */
	void setHitRate(double hitRate) { this->hitRate = hitRate; }
	/**
	 * @brief Get the total memory cost.
	 * @return The total memory cost.
	 */
	double getTotalMemoryCost() { return totalMemoryCost; }
	/**
	 * @brief Set the total memory cost.
	 * @param totalMemoryCost The total memory cost to set.
	 */
	void setTotalMemoryCost(double totalMemoryCost) { this->totalMemoryCost = totalMemoryCost; }

	/**
	 * @brief Get the results and costs as a map.
	 * @return A map of results and costs.
	 */
	map<string, double> getResultsAndCosts() {
		return {
			{"hitRate", hitRate},
			{"firstTableMissRate", firstTableMissRate},
			{"secondTableMissRate", secondTableMissRate},
			{"firstTableMemoryCost", firstTableMemoryCost},
			{"secondTableMemoryCost", secondTableMemoryCost},
			{"totalMemoryCost", totalMemoryCost},
		};
	}
};

/**
 * @brief Structure to store trace information.
 */
struct TraceInfo {
	std::string name; ///< Trace name.
	std::string filename; ///< Trace filename.
	unsigned long numAccesses; ///< Number of accesses.
};

/**
*  @brief Decodification function from XML to TraceInfo.	
* @param element XML element that contains the TraceInfo 
*/
TraceInfo decodeTraceInfo(TiXmlElement* element);


/**
 * @brief Template structure to store accesses dataset.
 * @tparam I Type of instruction.
 * @tparam A Type of access.
 */
template<typename I, typename A>
struct AccessesDataset {
	vector<A> accesses = vector<A>(); ///< Vector of accesses.
	vector<I> accessesInstructions = vector<I>(); ///< Vector of access instructions.
};

/**
 * @brief Template structure to store buffers dataset.
 * @tparam A Type of access.
 */
template<typename A = long>
struct BuffersDataset {
	vector<vector<A>> inputAccesses = vector<vector<A>>(); ///< Vector of input accesses.
	vector<A> outputAccesses = vector<A>(); ///< Vector of output accesses.
	vector<bool> isCacheMiss = vector<bool>(); ///< Vector indicating cache misses.
	vector<bool> isDictionaryMiss = vector<bool>(); ///< Vector indicating dictionary misses.
	vector<bool> isValid = vector<bool>(); ///< Vector indicating validity.
};
