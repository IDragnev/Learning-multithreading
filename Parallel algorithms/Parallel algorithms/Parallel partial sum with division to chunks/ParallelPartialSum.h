#pragma once

#include "Functional\Functional.h"
#include "SmartThread.h"
#include <iterator>
#include <numeric>
#include <future>
#include <vector>
#include <memory>

namespace IDragnev::Multithreading
{
	template <typename InputIt>
	class ParallelPartialSum
	{
	private:
		using T = typename std::iterator_traits<InputIt>::value_type;
		using Futures = std::vector<std::future<T>>;
		using Promises = std::vector<std::promise<T>>;
		using Threads = std::vector<SmartThread>;

		class PartialSumBlock
		{
		public:
			PartialSumBlock(InputIt start, InputIt end,
				            std::future<T>* previousBlockEndValue,
				            std::promise<T>* endValue);

			void operator()();

		private:
			bool hasPreviousBlock() const noexcept;
			void addPreviousBlockEndValueToAllValuesAndSetEndValuePromise();
			void addToLastValueAndSetEndValuePromise(const T& value);
			void setEndValuePromise();
			void addToAllValuesExceptTheLast(const T& value);
			bool isThisTheLastBlock() const noexcept;
			void propagateToNextBlock(std::exception_ptr e);

			static void partialSum(InputIt blockStart, InputIt blockEnd);

		private:
			InputIt blockStart;
			InputIt blockEnd;
			std::future<T>* previousBlockEndValue;
			std::promise<T>* blockEndValuePromise;
		};

	public:
		ParallelPartialSum() = default;
		ParallelPartialSum(const ParallelPartialSum& source) noexcept;
		~ParallelPartialSum() = default;

		ParallelPartialSum& operator=(const ParallelPartialSum& rhs) noexcept;

		void operator()(InputIt first, InputIt last);

	private:
		void initializeState(std::size_t length);
		void setNumberOfThreadsAndBlockSize(std::size_t length);
		void reserveMemory();
		InputIt splitWorkToThreads(InputIt first, InputIt last);
		void launchThread(InputIt blockStart, InputIt blockEnd, std::size_t blockNumber);
		void sumFinalBlock(InputIt blockStart, InputIt last);
		auto makeScopedClear() noexcept;

	private:
		static constexpr std::size_t MIN_ITEMS_PER_THREAD = 25;

	private:
		Threads threads;
		Futures endValueFutures;
		Promises endValuePromises;
		std::size_t numberOfThreads = 0;
		std::size_t blockSize = 0;
	};

	template <typename InputIt>
	inline void parallelPartialSum(InputIt first, InputIt last)
	{
		ParallelPartialSum<InputIt>{}(first, last);
	}
}

#include "PartialSumBlockImpl.hpp"
#include "ParallelPartialSumImpl.hpp"