
namespace IDragnev::Multithreading
{
	template <typename InputIt>
	ParallelPartialSum<InputIt>::PartialSumBlock::PartialSumBlock(InputIt first, InputIt last,
		                                                          std::future<T>* previousBlockEndValue,
		                                                          std::promise<T>* endValue) :
		blockStart{ first },
		blockEnd{ last }, 
		previousBlockEndValue{ previousBlockEndValue },
		blockEndValuePromise{ endValue }
	{
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::PartialSumBlock::operator()()
	{
		try
		{
			partialSum(blockStart, blockEnd);
			if (hasPreviousBlock())
			{
				addPreviousBlockEndValueToAllValuesAndSetEndValuePromise();
			}
			else
			{
				setEndValuePromise();
			}
		}
		catch (...)
		{
			if (!isThisTheLastBlock())
			{
				propagateToNextBlock(std::current_exception());
			}
			else
			{
				throw; //the last block is processed in the main thread
			}
		}
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumBlock::partialSum(InputIt blockStart, InputIt blockEnd)
	{
		std::partial_sum(blockStart, ++blockEnd, blockStart);
	}

	template <typename InputIt>
	inline bool ParallelPartialSum<InputIt>::PartialSumBlock::hasPreviousBlock() const noexcept
	{
		return previousBlockEndValue != nullptr;
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::PartialSumBlock::addPreviousBlockEndValueToAllValuesAndSetEndValuePromise()
	{
		decltype(auto) value = previousBlockEndValue->get();				
		addToLastValueAndSetEndValuePromise(value);
		addToAllValuesExceptTheLast(value);
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumBlock::addToLastValueAndSetEndValuePromise(const T& value)
	{
		*blockEnd += value;
		setEndValuePromise();
	}

	template <typename InputIt>
	void ParallelPartialSum<InputIt>::PartialSumBlock::setEndValuePromise()
	{
		if (!isThisTheLastBlock())
		{
			blockEndValuePromise->set_value(*blockEnd);
		}
	}

	template <typename InputIt>
	inline bool ParallelPartialSum<InputIt>::PartialSumBlock::isThisTheLastBlock() const noexcept
	{
		return blockEndValuePromise == nullptr;
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumBlock::addToAllValuesExceptTheLast(const T& value)
	{
		using Functional::plus;
		std::transform(blockStart, blockEnd, blockStart, plus(value));
	}

	template <typename InputIt>
	inline void ParallelPartialSum<InputIt>::PartialSumBlock::propagateToNextBlock(std::exception_ptr e)
	{
		blockEndValuePromise->set_exception(e);
	}
}