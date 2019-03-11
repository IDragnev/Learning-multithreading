#include <future>
#include <iostream>

using Signal = std::promise<void>;
using Barrier = std::shared_future<void>;
using namespace std::chrono_literals;

template <typename... Args>
void print(const Args&... args)
{
	(std::cout << ... << args);
}

void prepare()
{
	std::this_thread::sleep_for(1s);
}

auto makeTask(Barrier canStart, Signal ready)
{
	return [canStart, ready = std::move(ready)]() mutable
	{ 
		prepare();
		ready.set_value();
		canStart.wait();
		return 10;
	};
}

int main()
{
	Signal startSignal, firstReadySignal, secondReadySignal;
	Barrier canStart = startSignal.get_future();

	auto firstReady = firstReadySignal.get_future();
	auto secondReady = secondReadySignal.get_future();

	auto first = std::async(makeTask(canStart, std::move(firstReadySignal)));
	auto second = std::async(makeTask(canStart, std::move(secondReadySignal)));

	firstReady.wait();
	secondReady.wait();
	
	startSignal.set_value();

	print(first.get(), ' ', second.get(), "\n");
}


