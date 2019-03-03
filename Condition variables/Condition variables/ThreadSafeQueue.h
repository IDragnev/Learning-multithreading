#ifndef __THREAD_SAFE_QUEUE_H_INCLUDED__
#define __THREAD_SAFE_QUEUE_H_INCLUDED__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <type_traits>

namespace IDragnev
{
	namespace Threads
	{
		template <typename T>
		class ThreadSafeQueue
		{
			struct IsTypeRequirementSatisfied
			{
			private:
				static constexpr bool isNoThrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;
				static constexpr bool isNothrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;

			public:
				static constexpr bool value = isNoThrowMoveConstructible || (!isNoThrowMoveConstructible && isNothrowCopyConstructible);
			};

			static_assert(IsTypeRequirementSatisfied::value);

			using Queue = std::queue<T>;
			using LockGuard = std::lock_guard<std::mutex>;
			using UniqueLock = std::unique_lock<std::mutex>;

		public:
			ThreadSafeQueue() = default;
			ThreadSafeQueue(std::initializer_list<T> source);
			ThreadSafeQueue(const ThreadSafeQueue& other);
			~ThreadSafeQueue() = default;

			ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

			T waitAndExtractFront();
			void insertBack(T value);
			bool isEmpty() const;

		private:
			static Queue copyQueueOf(const ThreadSafeQueue& other);

			auto queueIsNotEmpty() const noexcept
			{
				return [this] { return !queue.empty(); };
			}

		private:
			mutable std::mutex mutex;
			std::condition_variable condition;
			Queue queue;
		};
	}
}

#include "ThreadSafeQueueImpl.hpp"
#endif // __THREAD_SAFE_QUEUE_H_INCLUDED__