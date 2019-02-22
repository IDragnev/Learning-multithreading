#include <numeric>
#include <algorithm>

namespace IDragnev
{
	namespace Threads
	{
		template <typename T, typename Iterator, typename BinaryOp>
		T ParallelAccumulate<T, Iterator, BinaryOp>::operator()(Iterator first, Iterator last, BinaryOp op, T initial)
		{
			if (auto length = std::distance(first, last);
				length != 0)
			{
				setNumberOfThreadsAndBlockSize(length);
				reserveMemory(initial);
				splitWorkToThreads(first, last, op);
				joinAllThreads();
				auto result = accumulateResults(std::move(op), std::move(initial));
				clear();

				return result;
			}
			else
			{
				return initial;
			}
		}

		template <typename T, typename Iterator, typename BinaryOp>
		void ParallelAccumulate<T, Iterator, BinaryOp>::setNumberOfThreadsAndBlockSize(std::size_t length)
		{
			auto maxThreads = (length + MIN_ITEMS_PER_THREAD - 1) / MIN_ITEMS_PER_THREAD;
			auto hardwareThreads = std::max(std::thread::hardware_concurrency(), 2u);

			numberOfThreads = std::min(hardwareThreads, maxThreads);
			blockSize = length / numberOfThreads;
		}

		template <typename T, typename Iterator, typename BinaryOp>
		inline void ParallelAccumulate<T, Iterator, BinaryOp>::reserveMemory(T& initial)
		{
			threads.reserve(numberOfThreads - 1);
			results = Results(numberOfThreads, initial);
		}

		template <typename T, typename Iterator, typename BinaryOp>
		void ParallelAccumulate<T, Iterator, BinaryOp>::splitWorkToThreads(Iterator first, Iterator last, BinaryOp op)
		{
			auto blockStart = first;
			auto indexOflast = numberOfThreads - 1;
			for (std::size_t i = 0; i < indexOflast; ++i)
			{
				auto blockEnd = blockStart;
				std::advance(blockEnd, blockSize);
				launchThread(blockStart, blockEnd, op, results[i]);
				blockStart = blockEnd;
			}

			accumulateBlock(blockStart, last, op, results[indexOflast]);
		}

		template <typename T, typename Iterator, typename BinaryOp>
		void ParallelAccumulate<T, Iterator, BinaryOp>::launchThread(Iterator first, Iterator last, BinaryOp op, T& result)
		{
			auto f = [first, last, op, &result, this] { accumulateBlock(first, last, op, result); };
			threads.emplace_back(std::thread{ f });
		}

		template <typename T, typename Iterator, typename BinaryOp>
		inline void ParallelAccumulate<T, Iterator, BinaryOp>::accumulateBlock(Iterator first, Iterator last, BinaryOp op, T& result)
		{
			result = std::accumulate(first, last, result, op);
		}

		template <typename T, typename Iterator, typename BinaryOp>
		inline void ParallelAccumulate<T, Iterator, BinaryOp>::joinAllThreads() 
		{
			threads.clear();
		}

		template <typename T, typename Iterator, typename BinaryOp>
		inline T ParallelAccumulate<T, Iterator, BinaryOp>::accumulateResults(BinaryOp op, T initial)
		{
			return std::accumulate(results.begin(), results.end(), initial, op);
		}

		template <typename T, typename Iterator, typename BinaryOp>
		inline void ParallelAccumulate<T, Iterator, BinaryOp>::clear()
		{
			results.clear();
			threads.clear();
		}
	}
}