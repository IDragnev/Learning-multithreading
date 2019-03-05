#include <iostream>
#include <future>
#include <numeric>

template <typename... Args>
void print(Args&&... args) 
{
	(std::cout << ... << args);
}

template <typename Iterator>
auto parallelSum(Iterator begin, Iterator end)
{
	using value_type = typename std::iterator_traits<Iterator>::value_type;

	if (auto length = end - begin;
		length < 1000)
	{
		return std::accumulate(begin, end, value_type(0));
	}
	else
	{
		auto middle = begin + (length / 2);
		auto secondHalf = std::async(std::launch::async, [middle, end]{ return parallelSum(middle, end); });
		auto firstHalf = parallelSum(begin, middle);

		return firstHalf + secondHalf.get();
	}
}

template <typename T>
auto sumNumbers(T from, T to)
{
	auto size = to - from + 1;
	std::vector<T> nums(size, 0);

	for (auto current = from; current <= to; ++current)
	{
		nums.push_back(current);
	}

	return parallelSum(nums.cbegin(), nums.cend());
}

int main()
{
	print("1 + ... + 10000 = ", sumNumbers(1u, 10'000u));
}
