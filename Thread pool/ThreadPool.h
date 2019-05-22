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
}
#endif //__THREAD_POOL_H_INCLUDED__
