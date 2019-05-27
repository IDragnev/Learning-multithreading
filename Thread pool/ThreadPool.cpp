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

	void ThreadPool::runPendingTask()
	{
		if (auto task = extractTask();
			task)
		{
			std::invoke(*task);
		}
		else
		{
			std::this_thread::yield();
		}
	}

	std::optional<Function> ThreadPool::extractTask()
	{
		if (auto task = extractTaskFromLocalQueue();
			task)
		{
			return task;
		}
		else
		{
			task = extractTaskFromGlobalQueue();
			return task ? std::move(task) : stealTaskFromOtherThread();
		}
	}

	std::optional<Function> ThreadPool::extractTaskFromLocalQueue()
	{
		return localQueue ? localQueue->extractFront() : std::nullopt;
	}

	std::optional<Function> ThreadPool::extractTaskFromGlobalQueue()
	{
		auto result = mainQueue.extractFront();			
		return result != nullptr ? std::move(*result) : std::nullopt; 
	}

	std::optional<Function> ThreadPool::stealTaskFromOtherThread()
	{
		auto queuesCount = threadLocalQueues.size();
		auto nextQueueIndex = 
			[queuesCount, this](auto i) { return localQueueIndex + i + 1 % queuesCount; };

		for (decltype(queuesCount) i = 0; i < queuesCount; ++i)
		{
			auto index = nextQueueIndex(i);

			if (auto result = threadLocalQueues[index]->extractBack();
				result != std::nullopt)
			{
				return result;
			}
		}

		return std::nullopt;
	}

	ThreadPool::~ThreadPool()
	{
		isDone = true;
	}
}