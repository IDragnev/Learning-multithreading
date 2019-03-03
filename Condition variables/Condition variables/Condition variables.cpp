#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include "ThreadSafeQueue.h"

using IDragnev::Threads::ThreadSafeQueue;
using Task = unsigned;
using TaskQueue = ThreadSafeQueue<Task>;

Task initialTask() 
{
	return 0u;
}

Task next(Task t)
{
	return ++t;
}

bool isTheLastTask(Task t)
{
	return t == 15u;
}

void loadTasks(TaskQueue& queue)
{
	auto task = initialTask();
	for (;;)
	{
		queue.insertBack(task);
		if (isTheLastTask(task)) { break; }
		task = next(task);
	}
}

void workOn(TaskQueue& queue)
{
	Task task;
	do
	{
		task = queue.waitAndExtractFront();
	} while (!isTheLastTask(task));
}

int main()
{ 
	TaskQueue tasks{};
	std::thread worker([&tasks] { workOn(tasks); });
	loadTasks(tasks);
	worker.join();
}

