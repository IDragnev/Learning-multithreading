#include <iostream>
#include <unordered_map>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <functional>
#include <cassert>
#include "..\..\Parallel Accumulate\Parallel Accumulate\SmartThread.h"

using IDragnev::Threads::SmartThread;

using Predicate = std::function<bool(int)>;

struct Entry 
{
	std::string value = "null";
};

std::ostream& operator<<(std::ostream& out, const Entry& e)
{
	out << e.value.c_str();
	return out;
}

class Cache
{
private:
	using Key = int;
	using Map = std::unordered_map<Key, Entry>;
	using Pair = Map::value_type;

	using WriterLock = std::lock_guard<std::shared_mutex>;
	using ReaderLock = std::shared_lock<std::shared_mutex>;

public:
	class ImmutableRangeView
	{
	private:
		friend class Cache;
		using ConstEntryRef = std::reference_wrapper<const Entry>;
		using Range = std::vector<ConstEntryRef>;
		using Lock = ReaderLock;

	public:
		auto begin() const { return range.begin(); }
		auto end() const { return range.end(); }

	private:
		ImmutableRangeView(Lock&& lock, Range&& range) :
			lock(std::move(lock)), 
			range(std::move(range))
		{
			assert(this->lock.owns_lock());
		}

	private:
		Lock lock;
		Range range;
	};

	class RangeView
	{
	private:
		friend class Cache;
		using EntryRef = std::reference_wrapper<Entry>;
		using Range = std::vector<EntryRef>;
		using Lock = std::unique_lock<std::shared_mutex>;

	public:
		auto begin() { return range.begin(); }
		auto end() { return range.end(); }

	private:
		RangeView(Lock&& lock, Range&& range) :
			lock(std::move(lock)),
			range(std::move(range))
		{
			assert(this->lock.owns_lock());
		}

	private:
		Lock lock;
		Range range;
	};

public:
	Cache(std::initializer_list<Pair> pairs) : map(pairs) { }
	~Cache() = default;

	Cache(const Cache&) = delete;
	Cache& operator=(const Cache&) = delete;

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

	auto viewAllReadOnly(Predicate p) const
	{
		return makeView<ImmutableRangeView>(mutex, map, p);
	}

	auto viewAll(Predicate p)
	{
		return makeView<RangeView>(mutex, map, p);
	}

private:
	template <typename View, typename Map>
	static auto makeView(std::shared_mutex& mutex, Map& map, Predicate p)
	{
		using Lock = typename View::Lock;
		using Range = typename View::Range;

		auto lock = Lock(mutex);
		auto range = Range{};

		for (auto&[key, entry] : map)
		{
			if (p(key))
			{
				range.emplace_back(entry);
			}
		}

		return View(std::move(lock), std::move(range));
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
	Cache cache{ { 1, {"e1"} }, { 2, {"e2"} }, { 3, {"e3"} }, { 4, {"e4"} } };
	auto threads = launchReaders(cache, { 1, 2, 3, 4, 5, 6 });

	cache.updateEntry(111, { "e4" });
	auto range = cache.viewAllReadOnly([](int key) { return key % 2 == 0; });
	
	joinAll(threads);

	for (const Entry& entry : range)
	{
		std::cout << entry << "\n";
	}

	return 0;
}

