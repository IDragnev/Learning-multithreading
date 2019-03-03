
namespace IDragnev
{
	namespace Threads
	{
		template <typename T>
		ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue& other) :
			queue(copyQueueOf(other))
		{
		}

		template <typename T>
		auto ThreadSafeQueue<T>::copyQueueOf(const ThreadSafeQueue& other) -> Queue
		{
			auto lock = LockGuard(other.mutex);
			return other.queue;
		}

		template <typename T>
		ThreadSafeQueue<T>::ThreadSafeQueue(std::initializer_list<T> source) :
			queue(source)
		{
		}

		template <typename T>
		T ThreadSafeQueue<T>::waitAndExtractFront()
		{
			auto lock = UniqueLock(mutex);
			condition.wait(lock, queueIsNotEmpty());

			auto result = std::move(queue.front());
			queue.pop();

			return result;
		}

		template <typename T>
		void ThreadSafeQueue<T>::insertBack(T value)
		{
			auto lock = LockGuard(mutex);
			queue.push(std::move(value));
			condition.notify_one();
		}

		template <typename T>
		bool ThreadSafeQueue<T>::isEmpty() const
		{
			auto lock = LockGuard(mutex);
			return queue.empty();
		}
	}
}