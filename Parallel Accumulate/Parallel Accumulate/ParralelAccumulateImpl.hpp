#include <numeric>
#include <algorithm>

namespace IDragnev::Multithreading
{
	template <typename T, typename Iterator, typename BinaryOp>
	T ParallelAccumulate<T, Iterator, BinaryOp>::operator()(Iterator first, Iterator last, T nullValue, T initialValue, BinaryOp op)
	{
		if (auto length = std::distance(first, last);
			length != 0)
		{
			auto clear = makeScopedClear();
			initializeState(length);
			
			auto lastBlockStart = splitWorkToThreads(first, last, nullValue, op);
			auto lastBlockResult = accumulateBlock(lastBlockStart, last, nullValue, op);
			
			return accumulateResults(initialValue, lastBlockResult, op);
		}
		else
		{
			return initialValue;
		}
	}

	template <typename T, typename Iterator, typename BinaryOp>
	auto ParallelAccumulate<T, Iterator, BinaryOp>::makeScopedClear()
	{
		auto deleter = [](auto ptr)
		{
			ptr->futures.clear();
			ptr->threads.clear();
			ptr->numberOfThreads = ptr->blockSize = 0;
		};
		
		using ScopedClear = std::unique_ptr<ParallelAccumulate, decltype(deleter)>;
		return ScopedClear{ this, deleter };
	}

	template <typename T, typename Iterator, typename BinaryOp>
	inline void ParallelAccumulate<T, Iterator, BinaryOp>::initializeState(std::size_t length)
	{
		setNumberOfThreadsAndBlockSize(length);
		reserveMemory();
	}

	template <typename T, typename Iterator, typename BinaryOp>
	void ParallelAccumulate<T, Iterator, BinaryOp>::setNumberOfThreadsAndBlockSize(std::size_t length)
	{
		auto maxThreads = (length + MIN_ITEMS_PER_THREAD - 1) / MIN_ITEMS_PER_THREAD;
		auto availableHardwareThreads = std::max(std::thread::hardware_concurrency(), 2u);

		numberOfThreads = std::min(availableHardwareThreads, maxThreads);
		blockSize = length / numberOfThreads;
	}

	template <typename T, typename Iterator, typename BinaryOp>
	void ParallelAccumulate<T, Iterator, BinaryOp>::reserveMemory()
	{
		auto size = numberOfThreads - 1;
		threads.reserve(size);
		futures.reserve(size);
	}

	template <typename T, typename Iterator, typename BinaryOp>
	Iterator ParallelAccumulate<T, Iterator, BinaryOp>::splitWorkToThreads(Iterator first, Iterator last, T nullValue, BinaryOp op)
	{
		auto blockStart = first;
		auto indexOflast = numberOfThreads - 1;

		for (decltype(indexOflast) i = 0; i < indexOflast; ++i)
		{
			auto blockEnd = blockStart;
			std::advance(blockEnd, blockSize);
			launchThread(blockStart, blockEnd, nullValue, op);
			blockStart = blockEnd;
		}

		return blockStart;
	}

	template <typename T, typename Iterator, typename BinaryOp>
	void ParallelAccumulate<T, Iterator, BinaryOp>::launchThread(Iterator first, Iterator last, T nullValue, BinaryOp op)
	{
		auto f = [first, last, nullValue, op] { return accumulateBlock(first, last, nullValue, op); };
		auto task = Task{ f };

		futures.emplace_back(task.get_future());
		threads.emplace_back(std::thread{ std::move(task) });
	}

	template <typename T, typename Iterator, typename BinaryOp>
	inline T ParallelAccumulate<T, Iterator, BinaryOp>::accumulateBlock(Iterator first, Iterator last, T nullValue, BinaryOp op)
	{
		return std::accumulate(first, last, nullValue, op);
	}

	template <typename T, typename Iterator, typename BinaryOp>
	T ParallelAccumulate<T, Iterator, BinaryOp>::accumulateResults(T initialValue, T lastBlockResult, BinaryOp op)
	{
		auto init = op(initialValue, lastBlockResult);
		auto operation = [op](auto&& result, auto& future) -> decltype(auto)
		{ 
			using T = decltype(result);
			return op(std::forward<T>(result), future.get());
		};

		return std::accumulate(std::begin(futures), std::end(futures), init, operation);
	}
}