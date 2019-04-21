#include "ParallelAccumulate.h"
#include <iostream>

namespace mt = IDragnev::Multithreading;

auto numsFromTo = [](auto from, auto to)
{
	std::vector<decltype(from)> result;

	for (auto i = from; i <= to; ++i)
	{
		result.push_back(i);
	}

	return result;
};

auto multiply = [](auto x, auto y) { return x * y; };

int main()
{
	auto nums = numsFromTo(1u, 10u);
	std::cout << "10! = " << mt::accumulate(std::cbegin(nums), std::cend(nums), 1u, 1u, multiply) << "\n";

	nums = numsFromTo(1u, 1000u);
	std::cout << "1 + ... + 1000 = " << mt::accumulate(std::cbegin(nums), std::cend(nums)) << std::endl;

	return 0;
}

