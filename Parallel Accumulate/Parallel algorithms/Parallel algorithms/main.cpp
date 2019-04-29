#include "ParallelFindIf.h"
#include <iostream>
#include <vector>
#include <chrono>

using IDragnev::Multithreading::parallelFindIf;

auto numsFromTo = [](auto from, auto to)
{
	std::vector<decltype(from)> result;

	for (auto i = from; i <= to; ++i)
	{
		result.push_back(i);
	}

	return result;
};

auto timeNow()
{
	return std::chrono::high_resolution_clock::now();
}

int main()
{
	auto nums = numsFromTo(1, 20'000);
	auto start = timeNow();
	auto it = parallelFindIf(std::cbegin(nums), std::cend(nums), [](auto x) { return x == 19'750; });
	auto duration = timeNow() - start;

	std::cout << std::boolalpha;
	std::cout << "Found? : " << (it != std::cend(nums));
	std::cout << "\nSearch took " << duration.count() << " nsec.";
}

