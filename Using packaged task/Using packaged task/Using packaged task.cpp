#include <iostream>
#include <future>

using Task = std::packaged_task<unsigned long()>;
using Tasks = std::vector<Task>;
using Result = std::future<unsigned long>;
using Results = std::vector<Result>;

template <typename... Args>
void print(Args&&... args)
{
	(std::cout << ... << args);
}

unsigned long pow(unsigned x, unsigned y)
{
	return std::pow(x, y);
}

void perform(Tasks& tasks)
{
	for (auto& t : tasks)
	{
		t();
	}
}

auto makeTasks()
{
	Tasks tasks;

	for (auto i = 0u; i <= 6u; ++i)
	{
		tasks.emplace_back([i] { return pow(i, 10u); });
	}

	return tasks;
}

auto futuresOf(Tasks& tasks)
{
	Results results;
	results.reserve(tasks.size());

	for (auto& task : tasks)
	{
		results.push_back(task.get_future());
	}

	return results;
}

void doSomeImportantWork()
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2s);
}

void process(Results& results)
{
	for (auto& result : results)
	{
		print(result.get(), "\n");
	}
}

int main()
{
	auto tasks = makeTasks();
	auto results = futuresOf(tasks);
	std::thread worker([&tasks] { perform(tasks); });
	doSomeImportantWork();
	worker.join();
	process(results);
}
