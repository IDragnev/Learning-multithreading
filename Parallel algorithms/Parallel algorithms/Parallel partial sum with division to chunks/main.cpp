#include "ParallelPartialSum.h"
#include <iostream>

using IDragnev::Multithreading::parallelPartialSum;

auto numsFromTo = [](auto from, auto to)
{
	std::vector<decltype(from)> result;

	for (auto i = from; i <= to; ++i)
	{
		result.push_back(i);
	}

	return result;
};

int main()
{
	auto nums = numsFromTo(1, 1000);
	parallelPartialSum(std::begin(nums), std::end(nums));

	for (auto n : nums)
	{
		std::cout << n << ' ';
	}
}

