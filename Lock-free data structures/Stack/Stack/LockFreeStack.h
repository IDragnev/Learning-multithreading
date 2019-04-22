#ifndef __LOCK_FREE_STACK__
#define __LOCK_FREE_STACK__

#define _ENABLE_ATOMIC_ALIGNMENT_FIX

#include <atomic>
#include <optional>
#include <assert.h>

namespace IDragnev::Multithreading
{
	template <typename T>
	class LockFreeStack
	{
	private:
		static_assert(std::is_nothrow_move_constructible_v<T>, 
			          "LockFreeStack cannot guarantee exception safety for T unles it is nothrow move-constructible");
		
		using CounterType = std::int32_t;

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
				next{ nullptr, 0 }
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

		template <typename... Args>
		void emplace(Args&&... args);
		void push(T&& item);
		void push(const T& item);
		std::optional<T> pop();

	private:
		template <typename... Args>
		void doPush(Args&&... args);
		void insertAsHead(RefCountedNodePtr ptr);

		static std::optional<T> extractDataOf(Node* node);
		static void updateRefCountAndFreeNodeIfNotReferenced(RefCountedNodePtr ptr);
		static void synchronizeWithRefCountUpdateAndFree(Node* node);

		template <typename... Args>
		static RefCountedNodePtr makeRefCountedNodePtr(Args&&... args);

		RefCountedNodePtr getHeadIncreasingItsRefCount(RefCountedNodePtr oldHead);

	private:
		std::atomic<RefCountedNodePtr> head;
	};
}

#include "LockFreeStackImpl.hpp"
#endif //__LOCK_FREE_STACK__