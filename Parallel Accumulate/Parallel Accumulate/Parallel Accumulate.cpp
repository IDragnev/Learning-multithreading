#include "ParallelAccumulate.h"
#include <iostream>

namespace IDT = IDragnev::Threads;

auto numsFromTo = [](auto from, auto to)
{
	std::vector<decltype(from)> result;

	for (; from <= to; ++from)
	{
		result.push_back(from);
	}

	return result;
};

auto multiply = [](auto x, auto y) { return x * y; };

auto sum = [](auto x, auto y) { return x + y; };

int main()
{
	using IDragnev::Threads::accumulate;

	auto nums = numsFromTo(1u, 10u);
	std::cout << "10! = " << IDT::accumulate(nums.begin(), nums.end(), multiply, 1u) << "\n";

	nums = numsFromTo(1u, 1000u);
	std::cout << "1 + ... + 1000 = " << IDT::accumulate(nums.begin(), nums.end(), sum, 0u) << std::endl;

	return 0;
}

