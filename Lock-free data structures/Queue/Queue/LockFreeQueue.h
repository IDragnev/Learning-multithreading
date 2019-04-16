#ifndef __LOCK_FREE_QUEUE_H_INCLUDED__
#define __LOCK_FREE_QUEUE_H_INCLUDED__

#include <atomic>
#include <cstdint>
#include <memory>

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

	public:
		void push(T item);
		std::unique_ptr<T> extractFront();

	private:
		RefCountedNodePtr getHeadIncreasingItsRefCount(RefCountedNodePtr oldHead);
		RefCountedNodePtr getTailIncreasingItsRefCount(RefCountedNodePtr oldTail);
		void setTail(RefCountedNodePtr& oldTail, const RefCountedNodePtr& newTail);
		Node* getTailNode() noexcept;

		template <typename Callable>
		void updateRefCountOf(Node* node, Callable update);

		static const RefCountedNodePtr emptyRefCountedNodePtr;

		static T* extractDataOf(Node* node) noexcept;
		static void releaseReferenceTo(Node* node);
		static void releaseExternalCounter(RefCountedNodePtr& ptr);
		static void deleteIfNotReferenced(Node* node, const RefCount& count) noexcept;
		static RefCountedNodePtr increaseExternalCount(AtomicRefCountedNodePtr& source, RefCountedNodePtr oldValue);

	private:
		AtomicRefCountedNodePtr head;
		AtomicRefCountedNodePtr tail;
	};
}

#include "LockFreeQueueImpl.hpp"
#endif //__LOCK_FREE_QUEUE_H_INCLUDED__