
namespace IDragnev::Multithreading
{
	template <typename T>
	LockFreeStack<T>::LockFreeStack() :
		head({ nullptr, 0 })
	{
	}

	template <typename T>
	LockFreeStack<T>::~LockFreeStack()
	{
		decltype(pop()) extracted = std::nullopt;
		do
		{
			extracted = pop();
		} while (extracted != std::nullopt);
	}
	
	template <typename T>
	inline void LockFreeStack<T>::push(const T& item)
	{
		doPush(item);
	}

	template <typename T>
	inline void LockFreeStack<T>::push(T&& item)
	{
		doPush(std::move(item));
	}

	template <typename T>
	template <typename Item>
	void LockFreeStack<T>::doPush(Item&& item)
	{
		auto ptr = makeRefCountedNodePtr(std::forward<Item>(item));
		insertAsHead(ptr);
	}

	template <typename T>
	template <typename... Args>
	auto LockFreeStack<T>::makeRefCountedNodePtr(Args&&... args) -> RefCountedNodePtr
	{
		return RefCountedNodePtr{ new Node(std::forward<Args>(args)...) };
	}

	template <typename T>
	void LockFreeStack<T>::insertAsHead(RefCountedNodePtr ptr)
	{
		ptr.node->next = head.load(std::memory_order_relaxed);
		while (!head.compare_exchange_weak(ptr.node->next, ptr,
			                               std::memory_order_release,
			                               std::memory_order_relaxed))
		{ }
	}

	template <typename T>
	std::optional<T> LockFreeStack<T>::pop()
	{
		auto oldHead = head.load(std::memory_order_relaxed);

		for (;;)
		{
			oldHead = getHeadIncreasingItsRefCount(oldHead);
			auto node = oldHead.node;

			if (!node)
			{
				return std::nullopt;
			}
			else if (head.compare_exchange_strong(oldHead, node->next, 
				                                  std::memory_order_relaxed))
			{
				auto result = extractDataOf(node);
				updateRefCountAndFreeNodeIfNotReferenced(oldHead);

				return result;
			}
			else if (auto oldCount = node->internalCount.fetch_sub(1, std::memory_order_relaxed);
				     oldCount == 1)
			{
				synchronizeWithRefCountUpdateAndFree(node);
			}
		}
	}

	template <typename T>
	auto LockFreeStack<T>::getHeadIncreasingItsRefCount(RefCountedNodePtr oldHead) -> RefCountedNodePtr
	{
		RefCountedNodePtr result;

		do
		{
			result = oldHead;
			++result.externalCount;
		} while (!head.compare_exchange_strong(oldHead, result,
											   std::memory_order_acquire,
											   std::memory_order_relaxed));

		return result;
	}

	template <typename T>
	inline std::optional<T> LockFreeStack<T>::extractDataOf(Node* node)
	{
		return std::move(node->data);
	}

	template <typename T>
	void LockFreeStack<T>::updateRefCountAndFreeNodeIfNotReferenced(RefCountedNodePtr ptr)
	{
		auto node = ptr.node;
		auto increase = ptr.externalCount - 2;

		if (auto oldCount = node->internalCount.fetch_add(increase, std::memory_order_release);
			oldCount == -increase)
		{
			delete node;
		}
	}

	template <typename T>
	inline void LockFreeStack<T>::synchronizeWithRefCountUpdateAndFree(Node* node)
	{
		auto x = node->internalCount.load(std::memory_order_acquire);
		delete node;
	}

}