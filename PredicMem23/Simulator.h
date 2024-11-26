/////////////////////////////////////////////////////////////////////////////////
/// @file Simulator.h
/// @brief Definition of the Simulator class template.
///
/// This file contains the definition of the Simulator class template, which
/// serves as a base class for different types of simulators in the PredicMem23 project.
///
/// @details
/// The following class template is included:
/// - Simulator
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
/// This file declares the Simulator class template, which serves as a base
/// class for different types of simulators in the PredicMem23 project.
/// The class template defines the interface for simulating and cleaning up resources.
/////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Base class template for simulators.
 *
 * @tparam T Type of tag.
 * @tparam I Type of instruction.
 * @tparam A Type of access.
 * @tparam LA Type of last access.
 */
#pragma once
template<typename T, typename I, typename A, typename LA>
class Simulator {
public:

	/**
	 * @brief Clean up the simulator resources.
	 */
	void clean() = 0;
	/**
	 * @brief Simulate the accesses dataset.
	 *
	 * @param dataset The dataset of accesses to simulate.
	 * @return A BuffersDataset containing the results of the simulation.
	 */
	BuffersDataset<A> simulate(AccessesDataset<I, LA> dataset) = 0;
};