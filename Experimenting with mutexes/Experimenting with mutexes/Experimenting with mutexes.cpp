#include "..\..\Parallel Accumulate\Parallel Accumulate\SmartThread.h"
#include <iostream>
#include <mutex>
#include <vector>

using IDragnev::Threads::SmartThread;

auto generateSequence = [](auto from, auto to)
{
	auto result = std::vector<decltype(from)>{};
	
	while (from <= to)
	{
		result.push_back(from++);
	}

	return result;
};

auto take = [](auto n)
{
	return [n](const auto& container)
	{
		using Result = std::decay_t<decltype(container)>;
		return Result(std::begin(container), std::begin(container) + n);
	};
};

using Sequence = std::vector<int>;

class Data
{
private:
	using Guard = std::lock_guard<std::mutex>;

public:
	void pushBack(Sequence&& s)
	{
		auto g = Guard(mutex);
		data.push_back(std::move(s));
	}

	auto extractAll() &&
	{ 
		auto g = Guard(mutex);
		return std::move(data);
	}

	auto extractLatest()
	{
		auto g = Guard(mutex);

		auto latest = data.back();
		data.pop_back();

		return latest;
	}

	void swap(Data& rhs)
	{
		if (this != &rhs)
		{
			auto lock = std::scoped_lock(mutex, rhs.mutex);
			std::swap(data, rhs.data);
		}
	}

private:
	std::mutex mutex;
	std::vector<Sequence> data;
};

using Range = std::pair<int, int>;

void joinAll(std::vector<SmartThread>& threads)
{
	threads.clear();
}

auto generateSequences = [](const auto& ranges)
{
	Data data;
	std::vector<SmartThread> threads;
	auto generate = [&data](auto from, auto to) { data.pushBack(generateSequence(from, to)); };

	for (auto [from, to] : ranges)
	{
		threads.emplace_back(std::thread{ generate, from, to });
	}

	joinAll(threads);

	return std::move(data).extractAll();
};

void print(const std::vector<Sequence>& sequences)
{
	for (const auto& s : sequences)
	{
		for (auto num : s)
		{
			std::cout << num << ' ';
		}
		std::cout << std::endl;
	}
}

int main()
{
	std::vector<Range> ranges{ {1,100}, {30, 66}, {22, 56}, {11, 54}, {43, 50}, {1, 10}, {11, 44} };
	auto sequences = generateSequences(ranges);
	print(sequences);
}
