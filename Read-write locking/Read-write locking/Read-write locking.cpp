#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <functional>
#include "..\..\Parallel Accumulate\Parallel Accumulate\SmartThread.h"

using IDragnev::Threads::SmartThread;

class Entry { };

class Cache
{
private:
	using Key = int;
	using Map = std::unordered_map<Key, Entry>;
	using Pair = Map::value_type;
	using Predicate = std::function<bool(int)>;
	using ConstPairRef = std::reference_wrapper<const Pair>;
	using Range = std::vector<ConstPairRef>;

	using WriterGuard = std::lock_guard<std::shared_mutex>;
	using ReaderGuard = std::shared_lock<std::shared_mutex>;

public:
	class RangeProtector
	{
	private:
		friend class Cache;
		using ConstIterator = Range::const_iterator;

	public:
		auto begin() const { return range.begin(); }
		auto end() const { return range.end(); }

	private:
		RangeProtector(ReaderGuard&& lock, Range&& range) :
			lock(std::move(lock)), 
			range(std::move(range))
		{
			std::cout << std::this_thread::get_id() << " getting a range!\n";
		}

	private:
		ReaderGuard lock;
		Range range;
	};

	Cache(std::initializer_list<Pair> pairs) : map(pairs) { }
	
	void updateEntry(const Key& key, const Entry& entry)
	{
		std::cout << std::this_thread::get_id() << " trying to write..\n";
		auto g = WriterGuard(mutex);

		std::cout << std::this_thread::get_id() << " writing!\n";
		map[key] = entry;

		std::cout << std::this_thread::get_id() << " is done writing..\n";
	}

	auto findEntry(const Key& key) const 
	{
		std::cout << std::this_thread::get_id() << " trying to read..\n";
		auto g = ReaderGuard(mutex);

		std::cout << std::this_thread::get_id() << " reading!\n";
		auto iterator = map.find(key);

		std::cout << std::this_thread::get_id() << " is done reading..\n";
		return (iterator != map.cend()) ? iterator->second : Entry{};
	}

	auto findAll(Predicate p) const
	{
		std::cout << std::this_thread::get_id() << " trying to read a range..\n";
		auto lock = ReaderGuard(mutex);

		std::cout << std::this_thread::get_id() << " preparing a range..\n";
		Range result;	
		for (const auto& pair : map)
		{
			if (p(pair.first))
			{ 
				result.emplace_back(pair);
			}
		}

		return RangeProtector(std::move(lock), std::move(result));
	}

private:
	mutable std::shared_mutex mutex;
	Map map;
};

auto launchReaders(const Cache& cache, std::initializer_list<int> keys)
{
	std::vector<SmartThread> readers;
	readers.reserve(keys.size());

	for (auto key : keys)
	{
		readers.emplace_back(std::thread{ [&cache, key] { cache.findEntry(key); } });
	}

	return readers;
}

auto joinAll = [](auto& threads) 
{
	threads.clear();
};

int main()
{
	Cache cache{ { 1, {} }, { 2, {} }, { 3, {} } };
	auto threads = launchReaders(cache, { 1,2,3,4,5,6 });

	cache.updateEntry(111, {});
	auto pairRange = cache.findAll([](int key) { return key % 2 == 0; });
	
	joinAll(threads);

	return 0;
}

