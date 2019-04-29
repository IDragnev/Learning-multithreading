#ifndef __PARALLEL_FIND_IF_INCLUDED__
#define __PARALLEL_FIND_IF_INCLUDED__

#include <atomic>
#include <future>

namespace IDragnev::Multithreading
{
	template <typename InputIt, typename Callable>
	class ParallelFindIf
	{
	public:
		ParallelFindIf() = default;
		ParallelFindIf(const ParallelFindIf& source) noexcept;
		~ParallelFindIf() = default;

		ParallelFindIf& operator=(const ParallelFindIf& rhs) noexcept;

		InputIt operator()(InputIt first, InputIt end, Callable match);

	private:
		InputIt findIf(InputIt first, InputIt last, Callable match);
		InputIt directSearch(InputIt first, InputIt last, Callable match);
		InputIt divideAndSearch(InputIt first, InputIt last, std::size_t length, Callable match);

		static bool isSmallEnough(std::size_t length);
		static constexpr std::size_t MIN_SUBRANGE_LENGTH = 25;

	private:
		std::atomic<bool> done = false;
	};

	template <typename InputIt, typename Callable>
	inline InputIt parallelFindIf(InputIt first, InputIt last, Callable match)
	{
		return ParallelFindIf<InputIt, Callable>{}(first, last, match);
	}
}

#include "ParallelFindIfImpl.hpp"
#endif //__PARALLEL_FIND_IF_INCLUDED__