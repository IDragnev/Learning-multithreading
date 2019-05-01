#pragma once

#include "Functional\Functional.h"
#include <iterator>
#include <numeric>
#include <future>

namespace IDragnev::Multithreading
{
	template <typename InputIt>
	class ParallelPartialSum
	{
	private:
		using T = typename std::iterator_traits<InputIt>::value_type;

		class PartialSumChunk
		{
		public:
			PartialSumChunk(InputIt start, InputIt end,
				            std::future<T>* previousChunkEndValue,
				            std::promise<T>* endValue);

			void operator()();

		private:
			void partialSumThisChunk();
			bool hasPreviousChunk() const noexcept;
			void addPreviousChunkEndValueToAllValuesAndSetEndValuePromise();
			void addToLastValueAndSetEndValuePromise(const T& value);
			void setEndValuePromise();
			void addToAllValuesExceptTheLast(const T& value);
			bool isThisTheLastChunk() const noexcept;
			void propagateToNextChunk(std::exception_ptr e);

			static void partialSum(InputIt first, InputIt last);

		private:
			InputIt chunkStart;
			InputIt chunkEnd;
			std::future<T>* previousChunkEndValue;
			std::promise<T>* chunkEndValuePromise;
		};

	public:
		void operator()(InputIt first, InputIt last);
	};
}

#include "ParallelPatialSumImpl.hpp"