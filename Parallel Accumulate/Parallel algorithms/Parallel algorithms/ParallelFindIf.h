#pragma once

#include <atomic>
#include <future>
#include "UtilityFunctions.h"

namespace IDragnev::Multithreading
{
	namespace Detail
	{
		template <typename InputIt, typename Callable>
		InputIt parallelFindIfImpl(InputIt first, InputIt last, Callable match, std::atomic<bool>& done)
		{
			using Utility::CallOnDestruction;
			auto clear = CallOnDestruction{ [&done]() noexcept { done = true; } };
			
			static const auto subrangeLength = 25u;

			if (auto length = std::distance(first, last);
				length < 2 * subrangeLength)
			{
				for (auto current = first;
					current != last && !done.load();
					++current;)
				{
					if (match(*current))
					{
						done = true;
						return current;
					}
				}

				return last;
			}
			else
			{
				auto middle = first;
				std::advance(middle, length / 2u);
				auto secondHalfCall = [middle, last, match, &done] { parallelFindIfImpl(middle, last, match, done); };

				auto seconHalfResult = std::async(secondHalfCall);
				auto firstHalfResult = parallelFindIfImpl(first, middle, match, done);

				return (firstHalfResult != middle) ? firstHalfResult : seconHalfResult.get();
			}
		}
	}

	template <typename InputIt, typename Callable>
	inline InputIt parallelFindIf(InputIt first, InputIt last, Callable match)
	{
		std::atomic<bool> done = false;
		return Detail::parallelFindIfImpl(first, last, match, done);
	}
}