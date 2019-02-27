#include <iostream>
#include <unordered_map>
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

	using WriterLock = std::lock_guard<std::shared_mutex>;
	using ReaderLock = std::shared_lock<std::shared_mutex>;

public:
	class RangeView
	{
	private:
		friend class Cache;

		using ConstEntryRef = std::reference_wrapper<const Entry>;
		using Range = std::vector<ConstEntryRef>;

	public:
		auto begin() const { return range.begin(); }
		auto end() const { return range.end(); }

	private:
		RangeView(ReaderLock&& lock, Range&& range) :
			lock(std::move(lock)), 
			range(std::move(range))
		{
			std::cout << std::this_thread::get_id() << " getting a range!\n";
		}

	private:
		ReaderLock lock;
		Range range;
	};

public:
	Cache(std::initializer_list<Pair> pairs) : map(pairs) { }
	
	void updateEntry(const Key& key, const Entry& entry)
	{
		std::cout << std::this_thread::get_id() << " trying to write..\n";
		auto lock = WriterLock(mutex);

		std::cout << std::this_thread::get_id() << " writing!\n";
		map[key] = entry;

		std::cout << std::this_thread::get_id() << " is done writing..\n";
	}

	auto findEntry(const Key& key) const 
	{
		std::cout << std::this_thread::get_id() << " trying to read..\n";
		auto lock = ReaderLock(mutex);

		std::cout << std::this_thread::get_id() << " reading!\n";
		auto iterator = map.find(key);

		std::cout << std::this_thread::get_id() << " is done reading..\n";
		return (iterator != map.cend()) ? iterator->second : Entry{};
	}

	auto viewAll(Predicate p) const
	{
		std::cout << std::this_thread::get_id() << " trying to read a range..\n";
		auto lock = ReaderLock(mutex);

		std::cout << std::this_thread::get_id() << " preparing a range..\n";
		RangeView::Range result;
		for (const auto& [key, entry] : map)
		{
			if (p(key))
			{ 
				result.emplace_back(entry);
			}
		}

		return RangeView(std::move(lock), std::move(result));
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
	auto range = cache.viewAll([](int key) { return key % 2 == 0; });
	
	joinAll(threads);

	for (const Entry& entry : range)
	{
		//..
	}

	return 0;
}

