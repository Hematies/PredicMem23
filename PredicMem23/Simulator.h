#pragma once
template<typename T, typename I, typename A, typename LA>
class Simulator {
public:

	void clean() = 0;
	BuffersDataset<A> simulate(AccessesDataset<I, LA> dataset) = 0;
};