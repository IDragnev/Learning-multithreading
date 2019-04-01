#include <iostream>
#include "ThreadSafeQueue.h"
#include "..\..\..\Parallel Accumulate\Parallel Accumulate\SmartThread.h"

struct Item { };
using Queue = IDragnev::Multithreading::ThreadSafeQueue<Item>;
using IDragnev::Threads::SmartThread;

template <auto N, typename Callable>
void callNTimes(Callable f)
{
	for (decltype(N) i = 0; i < N; ++i)
	{
		f();
	}
}

template <auto N>
void insertNItems(Queue& queue)
{
	callNTimes<N>([&queue] { queue.insertBack({}); });
}

template <auto N>
void processNItems(Queue& queue)
{
	callNTimes<N>([&queue] { auto item = queue.waitAndExtractFront(); });
}

template <auto... Counts>
auto launchThreadsToPopulate(Queue& queue)
{
	return std::make_tuple(SmartThread{ std::thread{[&queue] { insertNItems<Counts>(queue); } } } ...);
}

int main()
{
	Queue queue;
	auto threads = launchThreadsToPopulate<10, 15, 12>(queue);
	processNItems<20>(queue);
}
