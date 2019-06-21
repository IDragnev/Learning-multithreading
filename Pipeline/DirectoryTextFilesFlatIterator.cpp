#include "DirectoryTextFilesFlatIterator.h"
#include "Functional.h"
#include <assert.h>
#include <algorithm>

namespace fs = std::filesystem;

namespace IDragnev::Multithreading
{
	DirectoryTextFilesFlatIterator::DirectoryTextFilesFlatIterator(const std::string& path)
	{
		try
		{
			current = Iterator{ path };
			toNextTextFile();
		}
		catch (fs::filesystem_error&)
		{
			throw NoSuchDirectory{ path };
		}
	}

	void DirectoryTextFilesFlatIterator::toNextTextFile()
	{
		using Functional::matches;
		using namespace std::string_literals;

		auto isTextFile = matches(".txt"s, [](const auto& entry) { return entry.path().extension(); });
		current = std::find_if(current, end, isTextFile);
	}

	DirectoryTextFilesFlatIterator::operator bool() const
	{
		return current != end;
	}

	std::string DirectoryTextFilesFlatIterator::operator*() const
	{
		assert(*this);
		return (*current).path().string();
	}

	DirectoryTextFilesFlatIterator& DirectoryTextFilesFlatIterator::operator++()
	{
		assert(*this);

		++current;
		toNextTextFile();

		return *this;
	}
}

