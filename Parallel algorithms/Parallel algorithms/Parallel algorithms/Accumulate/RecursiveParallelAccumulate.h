
namespace IDragnev::Multithreading
{
	template <typename Iterator, typename T, typename BinaryOp>
	T recursiveParallelAccumulate(Iterator first, Iterator last, T nullValue, BinaryOp op)
	{
		static const auto maxChunkSize = 25ul;
		
		if (auto length = std::distance(first, last); 
			length <= maxChunkSize)
		{
			return std::accumulate(first, last, nullValue, op);
		}
		else
		{
			auto middle = std::next(first, length / 2);
			auto self = [=] { return recursiveParallelAccumulate(first, middle, nullValue, op); };

			auto firstHalf = std::async(self);
			auto secondHalf = recursiveParallelAccumulate(middle, last, nullValue, op);

			return op(firstHalf.get(), std::move(secondHalf));
		}
	}
}