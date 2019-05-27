#ifndef __THREAD_POOL_H_INCLUDED__
#define __THREAD_POOL_H_INCLUDED__

#include "Function.h"
#include "SmartThread.h"
#include "WorkStealableQueue.h"
#include "Lock-free data structures\Queue\Queue\LockFreeQueue.h"
#include <type_traits>

namespace IDragnev::Multithreading
{
	class ThreadPool
	{
	private:
		using WorkStealableQueuePtrs = std::vector<std::unique_ptr<WorkStealableQueue>>;
		using Threads = std::vector<SmartThread>;
		template <typename Callable>
		using TaskHandle = std::future<std::invoke_result_t<Callable>>;
		template <typename Callable>
		using TaskType = std::packaged_task<std::invoke_result_t<Callable>()>;

	public:
		ThreadPool();
		~ThreadPool();

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		void runPendingTask();

		template <typename Callable>
		TaskHandle<Callable> submit(Callable task);

	private:
		void launchWorkerThreads();
		void work(std::size_t queueIndex);

		template <typename Callable>
		void store(TaskType<Callable>&& task);

		std::optional<Function> extractTask();
		std::optional<Function> extractTaskFromLocalQueue();
		std::optional<Function> extractTaskFromGlobalQueue();
		std::optional<Function> stealTaskFromOtherThread();

		static WorkStealableQueuePtrs makeLocalQueues(std::uint32_t count);

	private:
		void initializeThreadLocalState(std::size_t queueIndex) noexcept;

		static thread_local WorkStealableQueue* localQueue;
		static thread_local std::size_t localQueueIndex;

	private:
		std::atomic<bool> isDone;
		std::size_t numberOfThreads;
		LockFreeQueue<Function> mainQueue;
		WorkStealableQueuePtrs threadLocalQueues;
		Threads threads;
	};

	template <typename Callable>
	auto ThreadPool::submit(Callable f) -> TaskHandle<Callable>
	{
		using Task = TaskType<Callable>;

		auto task = Task(f);
		auto handle = task.get_future();

		store(std::move(task));

		return handle;
	}

	template <typename Callable>
	void ThreadPool::store(TaskType<Callable>&& task)
	{
		if (localQueue)
		{
			localQueue->insertFront(std::move(task));
		}
		else
		{
			mainQueue.enqueue(std::move(task));
		}
	}
}
#endif //__THREAD_POOL_H_INCLUDED__
