
namespace IDragnev::Multithreading
{
	template <typename T>
	inline auto LockFreeQueue<T>::getHeadIncreasingItsRefCount(RefCountedNodePtr oldHead) -> RefCountedNodePtr
	{
		return increaseExternalCount(head, oldHead);
	}

	template <typename T>
	inline auto LockFreeQueue<T>::getTailIncreasingItsRefCount(RefCountedNodePtr oldTail) -> RefCountedNodePtr
	{
		return increaseExternalCount(tail, oldTail);
	}

	template <typename T>
	auto LockFreeQueue<T>::increaseExternalCount(AtomicRefCountedNodePtr& source, RefCountedNodePtr oldValue) -> RefCountedNodePtr
	{
		RefCountedNodePtr result;

		do
		{
			result = oldValue;
			++result.externalCount;
		} while (!source.compare_exchange_strong(oldValue, result,
			      std::memory_order_acquire,
			      std::memory_order_relaxed));
		
		return result;
	}

	template <typename T>
	void LockFreeQueue<T>::releaseReferenceTo(Node* node)
	{
		updateRefCountOf(node, [](auto oldCount)
		{
			--oldCount.internalCount;
			return oldCount;
		});
	}

	template <typename T>
	void LockFreeQueue<T>::releaseExternalCounter(RefCountedNodePtr& ptr)
	{
		auto increase = ptr.externalCount - 2;
		
		updateRefCountOf(ptr.node, [increase](auto oldCount)
		{
			--oldCount.externalCounters;
			oldCount.internalCount += increase;
			return oldCount;
		});
	}

	template <typename T>
	template <typename Callable>
	void LockFreeQueue<T>::updateRefCountOf(Node* node, Callable update)
	{
		auto oldCount = node->count.load(std::memory_order_relaxed);
		decltype(oldCount) newCount;

		do
		{
			newCount = update(oldCount);
		} while (!node->count.compare_exchange_strong(
			       oldCount, newCount,
		   	       std::memory_order_acquire, std::memory_order_relaxed));

		deleteIfNotReferenced(node, newCount);
	}

	template <typename T>
	inline void LockFreeQueue<T>::deleteIfNotReferenced(Node* node, const RefCount& count)
	{
		if (count.internalCount == 0 &&
			count.externalCounters == 0)
		{
			delete node;
		}
	}
}

