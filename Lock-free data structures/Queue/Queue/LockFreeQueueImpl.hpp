
namespace IDragnev::Multithreading
{
	template <typename T>
	const LockFreeQueue<T>::RefCountedNodePtr LockFreeQueue<T>::emptyRefCountedNodePtr = { nullptr, 0 };

	template <typename T>
	std::unique_ptr<T> LockFreeQueue<T>::extractFront()
	{
		auto oldHead = head.load(std::memory_order_relaxed);
		for (;;)
		{
			oldHead = getHeadIncreasingItsRefCount(oldHead);
			auto node = oldHead.node;

			if (node == getTailNode())
			{
				return nullptr;
			}

			auto next = node->next.load();
			if (head.compare_exchange_strong(oldHead, next))
			{
				auto result = extractDataOf(node);
				releaseExternalCounter(oldHead);
				return result;
			}

			releaseReferenceTo(node);
		}
	}

	template <typename T>
	inline auto LockFreeQueue<T>::getHeadIncreasingItsRefCount(RefCountedNodePtr oldHead) -> RefCountedNodePtr
	{
		return increaseExternalCount(head, oldHead);
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
	inline auto LockFreeQueue<T>::getTailNode() noexcept -> Node*
	{
		return tail.load().node;
	}

	template <typename T>
	inline T* LockFreeQueue<T>::extractDataOf(Node* node) noexcept
	{
		return node->data.exchange(nullptr);
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
	inline void LockFreeQueue<T>::deleteIfNotReferenced(Node* node, const RefCount& count) noexcept
	{
		if (count.internalCount == 0 &&
			count.externalCounters == 0)
		{
			delete node;
		}
	}

	template <typename T>
	void LockFreeQueue<T>::push(T item)
	{
		auto newData = std::make_unique(std::move(item));
		auto newNext = RefCountedNodePtr{ new Node };
		auto oldTail = tail.load();

		for (;;)
		{
			oldTail = getTailIncreasingItsRefCount(oldTail);

			T* noData = nullptr;
			if (oldTail.node->data.compare_exchange_strong(noData, newData.get()))
			{
				auto oldNext = emptyRefCountedNodePtr;
				if (!oldTail.node->next.compare_exchange_strong(oldNext, newNext))
				{
					delete newNext.node;
					newNext = oldNext;
				}
				setTail(oldTail, newNext);
				newData.release();
				break;
			}
			else
			{
				auto oldNext = emptyRefCountedNodePtr;
				if (oldTail.node->next.compare_exchange_strong(oldNext, newNext))
				{
					oldNext = newNext;
					newNext.ptr = new Node;
				}
				setTail(oldTail, oldNext);
			}
		}
	}

	template <typename T>
	inline auto LockFreeQueue<T>::getTailIncreasingItsRefCount(RefCountedNodePtr oldTail) -> RefCountedNodePtr
	{
		return increaseExternalCount(tail, oldTail);
	}

	template <typename T>
	void LockFreeQueue<T>::setTail(RefCountedNodePtr& oldTail, const RefCountedNodePtr& newTail)
	{
		auto currentTailNode = oldTail.node;

		while (!tail.compare_exchange_weak(oldTail, newTail) &&
			   oldTail.node == currentTailNode)
		{ }

		if (oldTail.node == currentTailNode)
		{
			releaseExternalCounter(oldTail);
		}
		else
		{
			releaseReferenceTo(currentTailNode);
		}
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
}

