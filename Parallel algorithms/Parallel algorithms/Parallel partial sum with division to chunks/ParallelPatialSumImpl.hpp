
namespace IDragnev::Multithreading
{
	template <typename InputIt>
	ParallelPartialSum<InputIt>::PartialSumChunk::PartialSumChunk(InputIt first, InputIt last,
		                                                          std::future<T>* previousChunkEndValue,
		                                                          std::promise<T>* endValue) :
		chunkStart{ first },
		chunkEnd{ last }, 
		previousChunkEndValue{ previousChunkEndValue },
		chunkEndValuePromise{ endValue }
	{
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::PartialSumChunk::operator()()
	{
		try
		{
			partialSumThisChunk();
			if (hasPreviousChunk())
			{
				addPreviousChunkEndValueToAllValuesAndSetEndValuePromise();
			}
			else
			{
				setEndValuePromise();
			}
		}
		catch (...)
		{
			if (!isThisTheLastChunk())
			{
				propagateToNextChunk(std::current_exception());
			}
			else
			{
				throw; //the last chunk is processed in the main thread
			}
		}
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumChunk::partialSumThisChunk()
	{
		partialSum(chunkStart, chunkEnd);
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumChunk::partialSum(InputIt first, InputIt last)
	{
		std::partial_sum(first, ++last, first);
	}

	template <typename InputIt>
	inline bool ParallelPartialSum<InputIt>::PartialSumChunk::hasPreviousChunk() const noexcept
	{
		return previousChunkEndValue != nullptr;
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::PartialSumChunk::addPreviousChunkEndValueToAllValuesAndSetEndValuePromise()
	{
		decltype(auto) value = previousChunkEndValue->get();				
		addToLastValueAndSetEndValuePromise(value);
		addToAllValuesExceptTheLast(value);
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumChunk::addToLastValueAndSetEndValuePromise(const T& value)
	{
		*chunkEnd += value;
		setEndValuePromise();
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::PartialSumChunk::setEndValuePromise()
	{
		if (!isThisTheLastChunk())
		{
			chunkEndValuePromise->set_value(*chunkEnd);
		}
	}

	template <typename InputIt>
	inline bool ParallelPartialSum<InputIt>::PartialSumChunk::isThisTheLastChunk() const noexcept
	{
		return chunkEndValuePromise == nullptr;
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumChunk::addToAllValuesExceptTheLast(const T& value)
	{
		using Functional::plus;
		std::transform(chunkStart, chunkEnd, chunkStart, plus(value));
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumChunk::propagateToNextChunk(std::exception_ptr e)
	{
		chunkEndValuePromise->set_exception(e);
	}
}