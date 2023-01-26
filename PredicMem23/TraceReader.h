#pragma once
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
// #include "BuffersSimulator.h"

using namespace std;

template<typename I, typename O>
class TraceReader {
private:
public:
	string filename;
	ifstream file;

	TraceReader(string filename) {
		this->filename = filename;
		file = ifstream(filename);
		file.open(filename);
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
			if (line.compare("#eof") == 0) {
				break;
			}
			res++;
		}
		// file.close();
		// file.open(filename);
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
			file.seekg(0);
			while (file.peek() != EOF)
			{
				getline(file, line);

				// if ((k >= start) && (k < end)) {
				if ((k >= end) || (line.compare("#eof") == 0)) break;
				else if(k < end) {
					// Example of line:
					// 0x7f2974d88093: W 0x7ffeedfc8e88
					int index = line.find(delimiter);
					size_t dummy = 0;
					L64b instruction = stoll(line.substr(0, index), &dummy, 16);
					index += delimiter.size();
					aux = line.substr(index, line.size());
					int index_ = aux.find(space);
					L64b address = stoll(aux.substr(index_, aux.size() - index_), &dummy, 16);

					res.accessesInstructions.push_back(instruction);
					res.accesses.push_back(address);

					// currentLine++;
				}
				k++;
				
			}
		}
		// file.close();

		return res;
	}
};
