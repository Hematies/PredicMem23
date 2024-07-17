/////////////////////////////////////////////////////////////////////////////////
/// @file TraceReader.h
/// @brief Definition of functions dedicated to the reading and processing of instruction traces as datasets 
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
/// This file contains the implementation of various functions and methods
/// for reading and processing instruction traces into datasets. The provided
/// functionality includes reading all lines, reading a specified number of lines,
/// and reading lines between specified start and end positions.
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
// #include "BuffersSimulator.h"
#include<set>
#include<filesystem>

using namespace std;
namespace fs = std::filesystem;


/**
 * @brief Template class to read trace files.
 *
 * @tparam I Input type.
 * @tparam O Output type.
 */
template<typename I, typename O>
class TraceReader {
public:

	string filename = ""; ///< Name of the trace file.
	ifstream file = ifstream(); ///< File stream to read the trace file.
	unsigned long currentLine = 0; ///< Current line being read in the trace file.

	string endLine = "#eof"; ///< String that marks the end of the file.
	string skipLineHeader = "->"; ///< String that marks lines to be skipped.

	/** @brief Empty constructor of the TraceReader class.

	@param filename Name of the file that contains the desired trace.
	*/
	TraceReader() {
		this->filename = "";
	}

	/** @brief Constructor of the TraceReader class based on the name of the trace file.

	@param filename Name of the file that contains the desired trace.
	*/
	TraceReader(string filename) {
		this->filename = filename;
		file = ifstream(filename);
		file.open(filename);
	}
	
	/** @brief Constructor of the TraceReader class based on another TraceReader object.

	@param t Address of the TraceReader object to copy.
	*/
	TraceReader(const TraceReader<I,O>& t) {
		this->filename = t.filename;
		try {
			this->file = ifstream(filename);
		}
		catch(...){
			this->file = ifstream();
		}
		this->currentLine = t.currentLine;
		this->endLine = t.endLine;
		this->skipLineHeader = t.skipLineHeader;
	}

	/** @brief Copy the information of the this TraceReader on a given TraceReader object.

	@param t : Address of the TraceReader object to pass the information.
	*/
	void copy(TraceReader<I, O>& t) {
		t.filename = filename;
		t.file = ifstream(filename);
		t.file.open(filename);
	}

	/**
	 * @brief Assignment operator for the TraceReader class.
	 *
	 * @param t The TraceReader object to assign from.
	 * @return The assigned TraceReader object.
	 */
	TraceReader operator=(const TraceReader<I, O>& t) {
		return TraceReader(t);
	}

	/**
	 * @brief Destructor of the TraceReader class.
	 */
	~TraceReader() {
		closeFile();
	}

	/** @brief Close the file used to read the trace.

	*/
	void closeFile() {
		file.close();
	}

	/**
	 * @brief Count the number of lines in the file that contains the trace information.
	 *
	 * @return The number of lines in the file.
	 */
	unsigned long countNumLines() {
		// if (file.is_open()) file.close();
		// file.open(filename);
		file.clear();
		file.seekg(0);
		string line;
		unsigned long res = 0;
		// while (file.peek() != EOF)
		while (!file.eof())
		{
			getline(file, line);
			if (strstr(line.c_str(), skipLineHeader.c_str()) == NULL) {
				if (line.compare(endLine) == 0) {
					break;
				}
				res++;
			}
				
		}
		// file.close();
		// file.open(filename);
		file.clear();
		file.seekg(0);
		return res;
	}

	/**
	 * @brief Read all lines from the trace file.
	 *
	 * @return AccessesDataset<I, O> Object containing the read data.
	 */
	AccessesDataset<I, O> readAllLines() {
		return readNextLines(countNumLines());
	}

	/**
	 * @brief Read the next specified number of lines from the trace file.
	 *
	 * @param numLines Number of lines to read.
	 * @return AccessesDataset<I, O> Object containing the read data.
	 */
	AccessesDataset<I, O> readNextLines(unsigned long numLines) {
		string line;
		int k = 0;
		string delimiter = ": ", space = " ";
		string aux; 


		long start = 0, // currentLine, 
			end = numLines;// end = currentLine + numLines;

		AccessesDataset<I, O> res;

		if (file.is_open())
		{
			file.clear();
			// file.seekg(0);
			while (file.peek() != EOF)
			{
				getline(file, line);

				// if ((k >= start) && (k < end)) {
				if ((k >= end) || (line.compare(endLine) == 0)) break;
				else if(strstr(line.c_str(), skipLineHeader.c_str()) == NULL) {
					if (k < end) {
						// Example of line:
						// 0x7f2974d88093: W 0x7ffeedfc8e88
						int index = line.find(delimiter);
						size_t dummy = 0;
						L64bu instruction = stoll(line.substr(0, index), &dummy, 16);
						index += delimiter.size();
						aux = line.substr(index, line.size());
						int index_ = aux.find(space);
						L64bu address = stoll(aux.substr(index_, aux.size() - index_), &dummy, 16);
						bool isRead = aux[0] == 'R';

						res.accessesInstructions.push_back(instruction);
						res.accesses.push_back(address);
						

					}
					k++;
					currentLine++;
				}
				
			}
		}
		// file.close();

		return res;
	}

	/**
	 * @brief Read lines from the trace file between specified start and end positions.
	 *
	 * @param inclusiveStart The starting line number (inclusive).
	 * @param exclusiveEnd The ending line number (exclusive).
	 * @return AccessesDataset<I, O> Object containing the read data.
	 */
	AccessesDataset<I, O> readLines(unsigned long inclusiveStart, unsigned long exclusiveEnd) {
		
		// We reset the file pointer if necessary:
		if (currentLine > inclusiveStart) {
			file.clear();
			file.seekg(0);
		}

		// We move the pointer to the given position:
		readNextLines(inclusiveStart - currentLine);

		// We read the lines that are between the start and the end:
		return readNextLines(exclusiveEnd - inclusiveStart);
	}
};
