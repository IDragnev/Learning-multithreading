#pragma once

#include "SmartThread.h"
#include <vector>
#include <future>
#include <memory>

namespace IDragnev::Multithreading
{
	inline auto sum = [](auto x, auto y) noexcept { return x + y; };

	template <typename T, typename Iterator, typename BinaryOp> 
	class ParallelAccumulate
	{
	private:
		using Threads = std::vector<SmartThread>;
		using Futures = std::vector<std::future<T>>;
		using Task = std::packaged_task<T()>;

	public:
		ParallelAccumulate() = default;
		ParallelAccumulate(const ParallelAccumulate& source) noexcept;
		~ParallelAccumulate() = default;

		ParallelAccumulate& operator=(const ParallelAccumulate& rhs) noexcept;

		T operator()(Iterator first, Iterator last, T nullValue, BinaryOp op);

	private:
		void initializeState(std::size_t length);
		void setNumberOfThreadsAndBlockSize(std::size_t length);
		void reserveMemory();
		Iterator splitWorkToThreads(Iterator first, Iterator last, T nullValue, BinaryOp op);
		void launchThread(Iterator first, Iterator last, T nullValue, BinaryOp op);
		T accumulateResults(T nullValue, T lastBlockResult, BinaryOp op);
		auto makeScopedClear();

		static T accumulateBlock(Iterator first, Iterator last, T nullValue, BinaryOp op);

	private:
		static constexpr std::size_t MIN_ITEMS_PER_THREAD = 25;

	private:
		Futures futures;
		Threads threads;
		std::size_t numberOfThreads = 0;
		std::size_t blockSize = 0;
	};

	template <typename Iterator,
		      typename T = typename std::iterator_traits<Iterator>::value_type,
		      typename BinaryOp = decltype(sum)
	> inline T accumulate(Iterator first, Iterator last, T nullValue = {}, BinaryOp op = sum)
	{
		using Accumulator = ParallelAccumulate<T, Iterator, BinaryOp>;
		return Accumulator{}(first, last, nullValue, op);
	}

	template <typename Iterator,
	 	      typename T = typename std::iterator_traits<Iterator>::value_type,
		      typename BinaryOp = decltype(sum)
	> T recursiveParallelAccumulate(Iterator first, Iterator last, T nullValue = {}, BinaryOp op = sum);
}

#include "ParralelAccumulateImpl.hpp"
#include "RecursiveParallelAccumulate.h"