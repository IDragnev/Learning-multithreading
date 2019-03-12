#include <future>
#include <iostream>

using Promise = std::promise<void>;
using SharedBarrier = std::shared_future<void>;
using TimePoint = std::chrono::steady_clock::time_point;
using namespace std::chrono_literals;

template <typename... Args>
void print(const Args&... args)
{
	(std::cout << ... << args);
}

auto timeNow()
{
	return std::chrono::steady_clock::now();
}

void prepare()
{
	std::this_thread::sleep_for(1s);
}

auto makeTask(TimePoint start, SharedBarrier canStart, Promise ready)
{
	return [start, canStart, ready = std::move(ready)]() mutable 
	{ 
		prepare();
		ready.set_value();
		canStart.wait();
		return timeNow() - start;
	};
}

template <typename... Promises>
auto futuresOf(Promises&... promises)
{
	return std::make_tuple(promises.get_future()...);
}

template <typename... Promises>
auto launchAsync(SharedBarrier canStart, Promises&&... promises)
{
	return std::make_tuple(std::async(std::launch::async, makeTask(timeNow(), canStart, std::move(promises)))...);
}

int main()
{
	Promise startPromise, firstReadyPromise, secondReadyPromise;
	SharedBarrier canStart = startPromise.get_future();

	auto[firstReady, secondReady] = futuresOf(firstReadyPromise, secondReadyPromise);
	auto[delayOfFirst, delayOfSecond] = launchAsync(canStart, std::move(firstReadyPromise), std::move(secondReadyPromise));
	
	firstReady.wait();
	secondReady.wait();
	
	startPromise.set_value();
	
	print("First thread started after ", delayOfFirst.get().count(), "ns\n");
	print("Second thread started after ", delayOfSecond.get().count(), "ns\n");
}


