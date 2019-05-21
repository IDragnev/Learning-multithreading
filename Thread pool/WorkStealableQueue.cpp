#include "WorkStealableQueue.h"

namespace IDragnev::Multithreading
{
	void WorkStealableQueue::insertFront(Function f)
	{
		auto lock = LockGuard(mutex);
		queue.push_front(std::move(f));
	}

	std::optional<Function> WorkStealableQueue::extractFront()
	{
		return extract(&WorkStealableQueue::doExtractFront);
	}

	std::optional<Function> WorkStealableQueue::extract(ExtractorFunction f)
	{
		auto lock = LockGuard(mutex);
		return !queue.empty() ? std::invoke(f, this) : std::nullopt;
	}

	std::optional<Function> WorkStealableQueue::doExtractFront()
	{
		auto f = std::move(queue.front());
		queue.pop_front();

		return f;
	}

	std::optional<Function> WorkStealableQueue::extractBack()
	{
		return extract(&WorkStealableQueue::doExtractBack);
	}

	std::optional<Function> WorkStealableQueue::doExtractBack()
	{
		auto f = std::move(queue.back());
		queue.pop_back();

		return f;
	}

	bool WorkStealableQueue::isEmpty() const
	{
		auto lock = LockGuard(mutex);
		return queue.empty();
	}
}