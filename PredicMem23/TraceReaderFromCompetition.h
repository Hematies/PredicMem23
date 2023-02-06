#pragma once
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include "TraceReader.h"

using namespace std;

class TraceReaderFromCompetition : public TraceReader<L64b,L64b> {
private:
public:
	string filename;
	ifstream file;
	unsigned long currentLine = 0;
	AccessesDataset<L64b, L64b> readNextLines(unsigned long numLines) {
		string line;
		int k = 0;
		string delimiter = ", ";
		string aux;


		long start = 0, // currentLine, 
			end = numLines;// end = currentLine + numLines;

		AccessesDataset<L64b, L64b> res;

		size_t dummy = 0;

		if (file.is_open())
		{
			file.clear();
			// file.seekg(0);
			while (file.peek() != EOF)
			{
				getline(file, line);

				// if ((k >= start) && (k < end)) {
				if ((k >= end) || (line.compare("#eof") == 0)) break;
				else if (k < end) {
					// Example of line:
					// 5, 255, 28e837c870c0, 406383, 0
					int index = line.find(delimiter);
					index += delimiter.size();
					aux = line.substr(0, index);

					index = aux.find(delimiter);
					index += delimiter.size();
					aux = aux.substr(0, index);

					index = aux.find(delimiter);
					L64b address = stoll(aux.substr(0, index), &dummy, 16);

					index += delimiter.size();
					aux = aux.substr(0, index);
					index = aux.find(delimiter);

					L64b instruction = stoll(line.substr(0, index), &dummy, 16);

					res.accessesInstructions.push_back(instruction);
					res.accesses.push_back(address);

				}
				k++;
				currentLine++;

			}
		}
		// file.close();

		return res;
	}

};
#pragma once
