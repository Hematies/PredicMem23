/////////////////////////////////////////////////////////////////////////////////
/// @file PredictorModel.h
/// @brief Definition of the PredictorModel class template.
///
/// This file contains the definition of the PredictorModel class template, which
/// serves as a base class for different types of predictor models in the PredicMem23 project.
///
/// @details
/// The following class template is included:
/// - PredictorModel
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
/// This file declares the PredictorModel class template, which serves as a base
/// class for different types of predictor models in the PredicMem23 project.
/// The class template defines the interface for simulating the model, importing data,
/// and cleaning up resources.
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Global.h"

/**
 * @brief Base class template for predictor models.
 *
 * @tparam T_access Type of access.
 * @tparam T_class Type of class.
 */
//template<typename T_access, typename T_delta, typename T_class>
template<typename T_access, typename T_class>
class PredictorModel {
public:
    /**
     * @brief Simulate the predictor model.
     *
     * @param initialize Flag to indicate whether to initialize the simulation.
     * @return A shared pointer to the results and costs of the simulation.
     */
	virtual shared_ptr<PredictResultsAndCosts> simulate(bool initialize = true) = 0;
    /**
     * @brief Import data into the predictor model.
     *
     * @param data Dataset of accesses.
     * @param datasetClases Dataset of buffer classes.
     */
	virtual void importData(AccessesDataset<T_access, T_access>& data, BuffersDataset<T_class>& datasetClases) = 0;
    /**
     * @brief Clean up the predictor model resources.
     */
	virtual void clean() = 0;
};