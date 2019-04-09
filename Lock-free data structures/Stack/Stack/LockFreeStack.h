#ifndef __LOCK_FREE_STACK
#define __LOCK_FREE_STACK

#include <atomic>
#include <optional>
#include <assert.h>

namespace IDragnev::Multithreading
{
	template <typename T>
	struct IsSafelyReturnable
	{
	private:
		static constexpr bool isNoThrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;
		static constexpr bool isNothrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;

	public:
		static constexpr bool value = isNoThrowMoveConstructible || (!isNoThrowMoveConstructible && isNothrowCopyConstructible);
	};

	template <typename T>
	static inline constexpr auto isSafelyReturnable = IsSafelyReturnable<T>::value;

	template <typename T>
	class LockFreeStack
	{
	private:
		static_assert(isSafelyReturnable<T>);
		
		using CounterType = std::uint32_t;

		struct Node;

		struct RefCountedNodePtr
		{
			Node* node = nullptr;
			CounterType externalCount = 1;
		};

		struct Node
		{
			template <typename... Args>
			Node(Args&&... args) :
				data(std::forward<Args>(args)...),
				internalCount(0),
				next(nullptr)
			{
			}

			T data;
			std::atomic<CounterType> internalCount;
			RefCountedNodePtr next;
		};

	public:
		~LockFreeStack()
		{
			auto extracted = std::nullopt;
			do
			{
				extracted = pop();
			} while (extracted != std::nullopt);
		}

		void push(const T& item)
		{
			auto ptr = RefCountedNodePtr{ new Node(item) };
			ptr.node->next = head.load();
			while(!head.compare_exchange_weak(ptr.node->next, ptr))
			{ }
		}

		std::optional<T> pop()
		{
			auto result = std::nullopt;
			auto oldHead = head.load();

			for (;;)
			{
				increaseHeadCount(oldHead);

				auto node = oldHead.node;

				if (!node)
				{
					return result;
				}
				if (head.compare_exchange_strong(oldHead, node->next))
				{
					result = std::move(node->data);
					auto countIncrease = oldHead.externalCount - 2;

					if (auto oldCount = node->internalCount.fetch_add(countIncrease);
						oldCount == -countIncrease)
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

	private:
		void increaseHeadCount(RefCountedNodePtr& oldHead)
		{
			auto temp = RefCountedNodePtr{};

			do
			{
				temp = oldHead;
				++temp.externalCount;
			} while (!head.compare_exchange_strong(oldHead, temp);

			oldHead.externalCount = temp.externalCount;
		}

	private:
		std::atomic<RefCountedNodePtr> head = nullptr;
	};
}

#include "LockFreeStackImpl.hpp"
#endif //__LOCK_FREE_STACK