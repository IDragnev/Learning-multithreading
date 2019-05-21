#ifndef __WORK_STEALABLE_QUEUE__
#define __WORK_STEALABLE_QUEUE__

#include "Function.h"
#include <deque>
#include <mutex>
#include <future>
#include <optional>

namespace IDragnev::Multithreading
{
	class WorkStealableQueue
	{
	private:
		static_assert(std::is_nothrow_move_constructible_v<Function>);

		using LockGuard = std::lock_guard<std::mutex>;
		using ExtractorFunction = std::optional<Function>(WorkStealableQueue::*)();

	public:
		WorkStealableQueue() = default;
		WorkStealableQueue(const WorkStealableQueue&) = delete;
		~WorkStealableQueue() = default;

		WorkStealableQueue& operator=(const WorkStealableQueue&) = delete;

		void insertFront(Function f);
		std::optional<Function> extractFront();
		std::optional<Function> extractBack();

		bool isEmpty() const;

	private:
		std::optional<Function> extract(ExtractorFunction f);
		std::optional<Function> doExtractFront();
		std::optional<Function> doExtractBack();

	private:
		std::deque<Function> queue;
		mutable std::mutex mutex;
	};
}

#endif //__WORK_STEALABLE_QUEUE__