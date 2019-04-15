#ifndef __LOCK_FREE_QUEUE_H_INCLUDED__
#define __LOCK_FREE_QUEUE_H_INCLUDED__

#include <atomic>
#include <cstdint>

namespace IDragnev::Multithreading
{
	template <typename T>
	class LockFreeQueue
	{
	private:
		struct Node;

		struct RefCountedNodePtr
		{
			Node* node;
			std::int32_t externalCount = 1;
		};

		using AtomicRefCountedNodePtr = std::atomic<RefCountedNodePtr>;

		struct RefCount
		{
			std::uint32_t internalCount : 30;
			std::uint32_t externalCounters : 2;
		};

		struct Node
		{
			Node() :
				data{ nullptr },
				count{ 0, 2 },
				next{ { nullptr, 0 } }
			{
			}

			std::atomic<T*> data;
			std::atomic<RefCount> count;
			AtomicRefCountedNodePtr next;
		};

	private:
		RefCountedNodePtr getHeadIncreasingItsRefCount(RefCountedNodePtr oldHead);
		RefCountedNodePtr getTailIncreasingItsRefCount(RefCountedNodePtr oldTail);

		template <typename Callable>
		void updateRefCountOf(Node* node, Callable update);

		static void releaseReferenceTo(Node* node);
		static void releaseExternalCounter(RefCountedNodePtr& ptr);
		static void deleteIfNotReferenced(Node* node, const RefCount& count);
		static RefCountedNodePtr increaseExternalCount(AtomicRefCountedNodePtr& source, RefCountedNodePtr oldValue);

	private:
		AtomicRefCountedNodePtr head;
		AtomicRefCountedNodePtr tail;
	};
}

#include "LockFreeQueueImpl.hpp"
#endif //__LOCK_FREE_QUEUE_H_INCLUDED__