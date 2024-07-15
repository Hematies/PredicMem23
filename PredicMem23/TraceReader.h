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

#pragma once
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
// #include "BuffersSimulator.h"
#include<set>
#include<filesystem>

using namespace std;
namespace fs = std::filesystem;

template<typename I, typename O>
class TraceReader {
public:
	string filename = "";
	ifstream file = ifstream();
	unsigned long currentLine = 0;

	string endLine = "#eof";
	string skipLineHeader ="->";


	TraceReader() {
		this->filename = "";
	}

	TraceReader(string filename) {
		this->filename = filename;
		file = ifstream(filename);
		file.open(filename);
	}
	
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

	void copy(TraceReader<I, O>& t) {
		t.filename = filename;
		t.file = ifstream(filename);
		t.file.open(filename);
	}

	TraceReader operator=(const TraceReader<I, O>& t) {
		return TraceReader(t);
	}

	~TraceReader() {
		closeFile();
	}

	void closeFile() {
		file.close();
	}

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


	AccessesDataset<I, O> readAllLines() {
		return readNextLines(countNumLines());
	}

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
