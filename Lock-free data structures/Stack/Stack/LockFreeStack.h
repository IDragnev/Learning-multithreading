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

		struct Node
		{
			template <typename... Args>
			Node(Args&&... args) :
				data(std::forward<Args>(args)...)
			{
			}

			T data;
			Node* next = nullptr;
		};

	public:
		void push(const T& item)
		{
			auto node = new Node(item);

			node->next = head.load();
			while (!head.compare_exchange_weak(node->next, node)) 
			{ }
		}

		std::optional<T> pop()
		{
			++threadsCurrentlyPopping;

			auto oldHead = head.load();                                   // extract a function: 
			while (oldHead &&                                             // Node* extractHead();
				   !head.compare_exchange_weak(oldHead, oldHead->next))   //
			{ }                                                           //

			auto result = oldHead ? std::move(oldHead->data) : std::nullopt; 
			tryToReclaim(oldHead);

			return result;
		}

	private:
		void tryToReclaim(Node* node)
		{
			if (threadsCurrentlyPopping == 1)
			{
				auto chain = claimChainToDelete();

				if (auto remaining = --threadsCurrentlyPopping;
					remaining == 0)
				{
					clear(chain);
				}
				else if (chain)
				{
					chainPendingNodes(chain);
				}
				
				delete node;
			}
			else
			{
				chainSinglePendingNode(node);
				--threadsCurrentlyPopping;
			}
		}
		
		inline Node* claimChainToDelete()
		{
			return chainToDelete.exchange(nullptr);
		}

		static void clear(Node* chain)
		{
			auto current = chain;

			while (current)
			{
				auto next = current->next;
				delete current;
				current = next;
			}
		}

		inline void chainPendingNodes(Node* start)
		{
			chainPendingNodes(start, endOfChain(start));
		}

		void endOfChain(Node* start)
		{
			assert(start);
			
			auto current = start;
			
			while (current->next)
			{
				current = current->next;
			}

			return current;
		}

		void chainPendingNodes(Node* first, Node* last)
		{
			last->next = chainToDelete;
			while (!chainToDelete.compare_exchange_weak(last->next, first))
			{ }
		}

		void chainSinglePendingNode(Node* n) 
		{
			chainPendingNodes(n, n);
		}

	private:
		std::atomic<Node*> head = nullptr;
		std::atomic<Node*> chainToDelete = nullptr;
		std::atomic<std::uint32_t> threadsCurrentlyPopping = 0;
	};
}

#include "LockFreeStackImpl.hpp"
#endif //__LOCK_FREE_STACK