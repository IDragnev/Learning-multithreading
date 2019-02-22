#pragma once

#include "SmartThread.h"
#include <vector>

namespace IDragnev
{
	namespace Threads
	{
		template <typename T, typename Iterator, typename BinaryOp>
		class ParallelAccumulate
		{
		private:
			using Results = std::vector<T>;

		public:
			T operator()(Iterator first, Iterator last, BinaryOp op, T initial);

		private:
			void setNumberOfThreadsAndBlockSize(std::size_t length);
			void reserveMemory(T& initial);
			void splitWorkToThreads(Iterator first, Iterator last, BinaryOp op);
			void launchThread(Iterator first, Iterator last, BinaryOp op, T& result);
			void accumulateBlock(Iterator first, Iterator last, BinaryOp op, T& result);
			void joinAllThreads();
			T accumulateResults(BinaryOp op, T initial);
			void clear();

		private:
			static constexpr std::size_t MIN_ITEMS_PER_THREAD = 25;

		private:
			Results results;
			std::vector<SmartThread> threads;
			std::size_t numberOfThreads = 0;
			std::size_t blockSize = 0;
		};

		template <typename Iterator, typename BinaryOp, typename T>
		T accumulate(Iterator first, Iterator last, BinaryOp op, T init)
		{
			auto f = ParallelAccumulate<T, Iterator, BinaryOp>{};
			return f(first, last, op, init);
		}
	}
}

#include "ParralelAccumulateImpl.hpp"