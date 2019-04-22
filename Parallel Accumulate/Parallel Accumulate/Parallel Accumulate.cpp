#include "ParallelAccumulate.h"
#include <iostream>
#include <chrono>

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

inline auto currentTime()
{
	return std::chrono::high_resolution_clock::now();
}

template <typename... Args>
void print(const Args&... args)
{
	(std::cout << ... << args);
}

template <typename Callable>
void measure(Callable f)
{
	auto start = currentTime();
	auto result = f();
	auto elapsedTime = currentTime() - start;
	print(result, ". Time taken: ", elapsedTime.count(), "ns\n");
}

int main()
{
	auto nums = numsFromTo(1u, 10u);
	print("Computing 10! :\n");
	print(" - with manual thread management : ");
	measure([&nums] { return mt::accumulate(std::cbegin(nums), std::cend(nums), 1u, multiply); });
	print(" - with recursion and std::async: ");
	measure([&nums] { return mt::recursiveParallelAccumulate(std::cbegin(nums), std::cend(nums), 1u, multiply); });

	nums = numsFromTo(1u, 10'000u);
	print("Computing 1 + ... + 10 0000: \n");
	print(" - with manual thread management : ");
	measure([&nums] { return mt::accumulate(std::cbegin(nums), std::cend(nums)); });
	print(" - with recursion and std::async: ");
	measure([&nums] { return mt::accumulate(std::cbegin(nums), std::cend(nums)); });
	
	try
	{
		print("\nChecking exception safety of manual thread management...  ");
		mt::accumulate(std::cbegin(nums), std::cend(nums), 0, [](auto x, auto y) { if (x < 3) return y; else throw 1; });
	}
	catch (int)
	{
		print("Exception propagated safely");
	}

	return 0;
}

