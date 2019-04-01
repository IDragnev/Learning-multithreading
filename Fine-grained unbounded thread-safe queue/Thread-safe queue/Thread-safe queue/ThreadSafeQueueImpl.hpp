
namespace IDragnev
{
	namespace Multithreading
	{
		template <typename T>
		ThreadSafeQueue<T>::ThreadSafeQueue() :
			head(makeDummyNode()),
			tail(head.get())
		{
		}

		template <typename T>
		inline auto ThreadSafeQueue<T>::makeDummyNode()
		{
			return std::make_unique<Node>();
		}

		template <typename T>
		void ThreadSafeQueue<T>::insertBack(T item)
		{
			auto data = std::make_unique<T>(std::move(item));
			auto dummy = makeDummyNode();

			updateTail(std::move(data), std::move(dummy));

			condition.notify_one();
		}

		template <typename T>
		void ThreadSafeQueue<T>::updateTail(std::unique_ptr<T>&& data, std::unique_ptr<Node>&& dummy)
		{
			auto lock = LockGuard(tailMutex);
			
			auto newTail = dummy.get();
			tail->data = std::move(data);
			tail->next = std::move(dummy);
			tail = newTail;
		}

		template <typename T>
		std::unique_ptr<T> ThreadSafeQueue<T>::tryToExtractFront()
		{
			auto lock = LockGuard(headMutex);
			
			return !checkIsEmpty() ? std::move(extractHead()->data) : nullptr;
		}

		template <typename T>
		inline bool ThreadSafeQueue<T>::checkIsEmpty()
		{
			//assumes headMutex is locked!
			return head.get() == getTail();
		}

		template <typename T>
		auto ThreadSafeQueue<T>::getTail() -> Node*
		{
			auto lock = LockGuard(tailMutex);
			return tail;
		}

		template <typename T>
		auto ThreadSafeQueue<T>::extractHead() -> std::unique_ptr<Node>
		{
			auto oldHead = std::move(head);
			head = std::move(oldHead->next);
			
			return oldHead;
		}

		template <typename T>
		std::unique_ptr<T> ThreadSafeQueue<T>::waitAndExtractFront()
		{
			auto lock = waitWhileEmpty();
			auto oldHead = extractHead();
			
			return std::move(oldHead->data);
		}

		template <typename T>
		auto ThreadSafeQueue<T>::waitWhileEmpty() -> UniqueLock
		{
			auto lock = UniqueLock(headMutex);
			condition.wait(lock, [this] { return !checkIsEmpty(); });
			
			return std::move(lock);
		}

		template <typename T>
		bool ThreadSafeQueue<T>::isEmpty()
		{
			auto lock = LockGuard(headMutex);
			return checkIsEmpty();
		}
	}
}