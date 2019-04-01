#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <assert.h>

namespace IDragnev
{
	namespace Multithreading
	{
		template <typename T>
		class ThreadSafeQueue
		{
		private:
			struct Node
			{
				std::unique_ptr<T> data = nullptr;
				std::unique_ptr<Node> next = nullptr;
			};

			using LockGuard = std::lock_guard<std::mutex>;
			using UniqueLock = std::unique_lock<std::mutex>;

		public:
			ThreadSafeQueue();
			~ThreadSafeQueue() = default;

			ThreadSafeQueue(const ThreadSafeQueue&) = delete;
			ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

			std::unique_ptr<T> tryToExtractFront();
			std::unique_ptr<T> waitAndExtractFront();
			void insertBack(T item);

			bool isEmpty();

		private:
			static auto makeDummyNode();
			
			Node* getTail();
			void updateTail(std::unique_ptr<T>&& data, std::unique_ptr<Node>&& dummy);
			std::unique_ptr<Node> extractHead();
			
			UniqueLock waitWhileEmpty();
			bool checkIsEmpty();

		private:
			std::unique_ptr<Node> head;
			Node* tail;
			std::mutex headMutex;
			std::mutex tailMutex;
			std::condition_variable condition;
		};
	}
}

#include "ThreadSafeQueueImpl.hpp"