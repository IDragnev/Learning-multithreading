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

void perform(Task& t)
{
	t();
}

void performAll(Tasks& tasks)
{
	std::for_each(tasks.begin(), tasks.end(), perform);
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
	
	std::transform(tasks.begin(), 
		           tasks.end(),
		           std::back_inserter<Results>(results),
		           [](Task& t) { return t.get_future(); });
	
	return results;
}

void doSomeImportantWork()
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2s);
}

void process(Results& results)
{
	for (auto& r : results)
	{
		print(r.get(), "\n");
	}
}

int main()
{
	auto tasks = makeTasks();
	auto results = futuresOf(tasks);
	std::thread worker([&tasks] { performAll(tasks); });
	doSomeImportantWork();
	worker.join();
	process(results);
}
