#pragma once

#include <algorithm>
#include <future>

namespace IDragnev::Multithreading
{
	template <typename InputIt, typename Callable>
	void parallelForEach(InputIt first, InputIt last, Callable f)
	{
		static const auto subrangeLength = 25u;
		
		if (auto length = std::distance(first, last); 
			length < 2 * subrangeLength)
		{
			std::for_each(first, last, f);
		}
		else
		{
			auto middle = std::next(first, length / 2);
			auto firstHalfCall = [first, middle, f] { parallelForEach(first, middle, f); };
			
			auto firstHalfBarrier = std::async(firstHalfCall);
			parallelForEach(middle, last, f);

			firstHalfBarrier.wait();
		}
	}
}