#include "ThreadPool.h"

namespace IDragnev::Multithreading
{
	ThreadPool::ThreadPool() :
		isDone(false),
		numberOfThreads(std::thread::hardware_concurrency()),
		threadLocalQueues(makeLocalQueues(numberOfThreads))
	{
		try
		{
			launchWorkerThreads();
		}
		catch (...)
		{
			isDone = true;
			throw;
		}
	}

	auto ThreadPool::makeLocalQueues(std::uint32_t count) -> WorkStealableQueuePtrs
	{
		auto result = WorkStealableQueuePtrs{ count, nullptr };

		for (decltype(count) i = 0; i < count; ++i)
		{
			result[i] = std::make_unique<WorkStealableQueue>();
		}

		return result;
	}

	void ThreadPool::launchWorkerThreads()
	{
		threads.reserve(numberOfThreads);

		for (decltype(numberOfThreads) i = 0; 
			i < numberOfThreads;
			++i)
		{
			threads.emplace_back(std::thread{ [this, queueIndex = i] { work(queueIndex); } });
		}
	}

	void ThreadPool::work(std::size_t queueIndex)
	{
		initializeThreadLocalState(queueIndex);

		while (!isDone)
		{
			runPendingTask();
		}
	}

	void ThreadPool::initializeThreadLocalState(std::size_t queueIndex) noexcept
	{
		localQueueIndex = queueIndex;
		localQueue = threadLocalQueues[localQueueIndex].get();
	}

	ThreadPool::~ThreadPool()
	{
		isDone = true;
	}
}