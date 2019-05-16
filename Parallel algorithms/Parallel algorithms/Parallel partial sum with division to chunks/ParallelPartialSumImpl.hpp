
namespace IDragnev::Multithreading
{
	template <typename InputIt>
	ParallelPartialSum<InputIt>::ParallelPartialSum(const ParallelPartialSum&) noexcept :
		ParallelPartialSum{}
	{
	}

	template <typename InputIt>
	auto ParallelPartialSum<InputIt>::operator=(const ParallelPartialSum&) noexcept -> ParallelPartialSum&
	{
		return *this;
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::operator()(InputIt first, InputIt last)
	{
		if (auto length = std::distance(first, last);
			length > 0)
		{
			auto clear = makeScopedClear();
			initializeState(length);
			auto lastBlockStart = splitWorkToThreads(first, last);
			sumFinalBlock(lastBlockStart, last);
		}
	}

	template <typename InputIt>
	auto ParallelPartialSum<InputIt>::makeScopedClear() noexcept
	{
		auto deleter = [](auto ptr)
		{
			ptr->endValuePromises.clear();
			ptr->endValueFutures.clear();
			ptr->threads.clear();
			ptr->numberOfThreads = ptr->blockSize = 0;
		};

		using ScopedClear = std::unique_ptr<ParallelPartialSum, decltype(deleter)>;
		return ScopedClear{ this, deleter };
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::initializeState(std::size_t length)
	{
		setNumberOfThreadsAndBlockSize(length);
		reserveMemory();
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::setNumberOfThreadsAndBlockSize(std::size_t length)
	{
		auto maxThreads = (length + MIN_ITEMS_PER_THREAD - 1) / MIN_ITEMS_PER_THREAD;
		auto availableHardwareThreads = std::max(std::thread::hardware_concurrency(), 2u);

		numberOfThreads = std::min(availableHardwareThreads, maxThreads);
		blockSize = length / numberOfThreads;
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::reserveMemory()
	{
		auto size = numberOfThreads - 1;
		threads.reserve(size);
		endValueFutures.reserve(size);
		endValuePromises = Promises(size);
	}

	template <typename InputIt>
	InputIt ParallelPartialSum<InputIt>::splitWorkToThreads(InputIt first, InputIt last)
	{
		const auto numberOfSubthreads = numberOfThreads - 1;
		auto blockStart = first;

		for (decltype(numberOfThreads) i = 0; i < numberOfSubthreads; ++i)
		{
			auto blockEnd = std::next(blockStart, blockSize - 1);
			launchThread(blockStart, blockEnd, i);
			blockStart = ++blockEnd;
		}

		return blockStart;
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::launchThread(InputIt blockStart, InputIt blockEnd, std::size_t blockNumber)
	{
		auto previousBlockEndValue = (blockNumber > 0u) ? &endValueFutures[blockNumber - 1u] : nullptr;
		auto endValuePromise = &endValuePromises[blockNumber];
		auto sumBlock = PartialSumBlock{ blockStart, blockEnd, previousBlockEndValue, endValuePromise };
		
		endValueFutures.emplace_back(endValuePromises[blockNumber].get_future());
		threads.emplace_back(std::thread{ std::move(sumBlock) });
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::sumFinalBlock(InputIt blockStart, InputIt last)
	{
		auto blockEnd = std::next(blockStart, std::distance(blockStart, last) - 1);
		auto previousBlockEndValue = (numberOfThreads > 1u) ? &endValueFutures.back() : nullptr;
		auto endValuePromise = nullptr;

		PartialSumBlock{ blockStart, blockEnd, previousBlockEndValue, endValuePromise }();
	}
}