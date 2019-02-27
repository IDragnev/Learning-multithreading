#include <iostream>
#include <unordered_map>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <functional>
#include "..\..\Parallel Accumulate\Parallel Accumulate\SmartThread.h"

using IDragnev::Threads::SmartThread;

struct Entry 
{
	std::string value = "null";
};

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
		}

	private:
		ReaderLock lock;
		Range range;
	};

public:
	Cache(std::initializer_list<Pair> pairs) : map(pairs) { }
	
	void updateEntry(const Key& key, const Entry& entry)
	{
		auto lock = WriterLock(mutex);
		map[key] = entry;
	}

	auto findEntry(const Key& key) const 
	{
		auto lock = ReaderLock(mutex);
		auto iterator = map.find(key);
		return (iterator != map.cend()) ? iterator->second : Entry{};
	}

	auto viewAll(Predicate p) const
	{
		auto lock = ReaderLock(mutex);
		auto range = RangeView::Range{};

		for (const auto& [key, entry] : map)
		{
			if (p(key))
			{ 
				range.emplace_back(entry);
			}
		}

		return RangeView(std::move(lock), std::move(range));
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
	Cache cache{ { 1, {"e1"} }, { 2, {"e2"} }, { 3, {"e3"} } };
	auto threads = launchReaders(cache, { 1, 2, 3, 4, 5, 6 });

	cache.updateEntry(111, { "e4" });
	auto range = cache.viewAll([](int key) { return key % 2 == 0; });
	
	joinAll(threads);

	for (const Entry& entry : range)
	{
		std::cout << entry.value.c_str() << "\n";
	}

	return 0;
}

