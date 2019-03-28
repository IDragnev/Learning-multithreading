#include <iostream>
#include <atomic>
#include <thread>
#include <assert.h>
#include "..\..\Parallel Accumulate\Parallel Accumulate\SmartThread.h"

using IDragnev::Threads::SmartThread;

enum class DataStates : std::uint8_t
{
	raw,
	initialized,
	ready
};

std::atomic<int> data = 0;
std::atomic<DataStates> state = DataStates::raw;

void initializeData()
{
	data.store(123, std::memory_order_relaxed);
	state.store(DataStates::initialized, std::memory_order_release);
}

void waitAndSignal()
{
	auto expected = DataStates::initialized;

	while (!state.compare_exchange_strong(expected, 
		                                  DataStates::ready,
		                                  std::memory_order_acq_rel))
	{
		expected = DataStates::initialized;
	}
}

void processData()
{
	while (state.load(std::memory_order_acquire) != DataStates::ready)
	{ }
	assert(data.load(std::memory_order_relaxed) == 123);
}

template <typename... Funs>
auto launchThreads(Funs... funs)
{
	return std::make_tuple(SmartThread{ std::thread{funs} }...);
}

int main()
{
	auto threads = launchThreads(initializeData, waitAndSignal, processData);
}
