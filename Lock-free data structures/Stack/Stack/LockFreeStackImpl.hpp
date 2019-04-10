
namespace IDragnev::Multithreading
{
	template <typename T>
	LockFreeStack<T>::LockFreeStack() :
		head{ nullptr, 0 }
	{
	}

	template <typename T>
	LockFreeStack<T>::~LockFreeStack()
	{
		auto extracted = std::nullopt;
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
		auto ptr = RefCountedNodePtr{ new Node(std::forward<Item>(item)) };

		ptr.node->next = head.load();
		while (!head.compare_exchange_weak(ptr.node->next, ptr))
		{ }
	}

	template <typename T>
	std::optional<T> LockFreeStack<T>::pop()
	{
		auto result = std::nullopt;
		auto oldHead = head.load();

		for (;;)
		{
			auto oldHead = getHeadIncreasingItsRefCount(oldHead);
			auto node = oldHead.node;

			if (!node)
			{
				return result;
			}
			if (head.compare_exchange_strong(oldHead, node->next))
			{
				result = std::move(node->data);
				auto increase = oldHead.externalCount - 2;

				if (auto oldCount = node->internalCount.fetch_add(increase);
					oldCount == -increase)
				{
					delete node;
				}

				return result;
			}
			else if (auto oldCount = node->internalCount.fetch_sub(1);
				oldCount == 1)
			{
				delete node;
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
		} while (!head.compare_exchange_strong(oldHead, result);

		return result;
	}
}