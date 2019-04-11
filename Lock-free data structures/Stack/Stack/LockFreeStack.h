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
		static_assert(isSafelyReturnable<T>, "LockFreeStack cannot guarantee exception safety for T");
		
		using CounterType = std::uint32_t;

		struct Node;

		struct RefCountedNodePtr
		{
			Node* node;
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
		LockFreeStack();
		LockFreeStack(const LockFreeStack&) = delete;
		~LockFreeStack();
		
		LockFreeStack& operator=(const LockFreeStack&) = delete;

		void push(T&& item);
		void push(const T& item);
		std::optional<T> pop();

	private:
		template <typename Item>
		void doPush(Item&& item);
		
		template <typename... Args>
		static RefCountedNodePtr makeRefCountedNodePtr(Args&&... args);

		RefCountedNodePtr getHeadIncreasingItsRefCount(RefCountedNodePtr oldHead);

	private:
		std::atomic<RefCountedNodePtr> head;
	};
}

#include "LockFreeStackImpl.hpp"
#endif //__LOCK_FREE_STACK