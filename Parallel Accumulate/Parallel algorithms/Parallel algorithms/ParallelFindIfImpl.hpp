
namespace IDragnev::Multithreading
{
	template <typename InputIt, typename Callable>
	inline ParallelFindIf<InputIt, Callable>::ParallelFindIf(const ParallelFindIf&) noexcept
	{
	}

	template <typename InputIt, typename Callable>
	inline auto ParallelFindIf<InputIt, Callable>::operator=(const ParallelFindIf&) noexcept -> ParallelFindIf&
	{
		return *this;
	}

	template <typename InputIt, typename Callable>
	InputIt ParallelFindIf<InputIt, Callable>::operator()(InputIt first, InputIt last, Callable match)
	{		
		done = false;
		return findIf(first, last, match);
	}

	template <typename InputIt, typename Callable>
	InputIt ParallelFindIf<InputIt, Callable>::findIf(InputIt first, InputIt last, Callable match)
	{
		try
		{
			auto length = std::distance(first, last);
			return isSmallEnough(length) ? directSearch(first, last, match) : divideAndSearch(first, last, length, match);
		}
		catch (...)
		{
			done = true;
			throw;
		}
	}

	template <typename InputIt, typename Callable>
	inline bool ParallelFindIf<InputIt, Callable>::isSmallEnough(std::size_t length)
	{
		return length < 2 * MIN_SUBRANGE_LENGTH;
	}

	template <typename InputIt, typename Callable>
	InputIt ParallelFindIf<InputIt, Callable>::directSearch(InputIt first, InputIt last, Callable match)
	{
		for (auto current = first;
			current != last && !done.load();
			++current)
		{
			if (match(*current))
			{
				done = true;
				return current;
			}
		}

		return last;
	}

	template <typename InputIt, typename Callable>
	InputIt ParallelFindIf<InputIt, Callable>::divideAndSearch(InputIt first, InputIt last, std::size_t length, Callable match)
	{
		auto middle = first;
		std::advance(middle, length / 2u);
		auto secondHalfCall = [middle, last, match, this] { return findIf(middle, last, match); };

		auto secondHalfResult = std::async(secondHalfCall);
		auto firstHalfResult = findIf(first, middle, match);

		return (firstHalfResult != middle) ? firstHalfResult : secondHalfResult.get();
	}
}