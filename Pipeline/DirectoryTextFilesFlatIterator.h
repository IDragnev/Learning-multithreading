#ifndef __DIR_ITERATOR_H_INCLUDED__
#define __DIR_ITERATOR_H_INCLUDED__

#include <filesystem>
#include <string>

namespace IDragnev::Multithreading
{
	class NoSuchDirectory : std::runtime_error
	{
	public:
		NoSuchDirectory(const std::string& path) : 
			std::runtime_error{ "No such directory: " + path }
		{
		}
	};

	class DirectoryTextFilesFlatIterator
	{
	private:
		using Iterator = std::filesystem::directory_iterator;
		using Self = DirectoryTextFilesFlatIterator;

	public:
		DirectoryTextFilesFlatIterator() = default;
		explicit DirectoryTextFilesFlatIterator(const std::string& path);
		DirectoryTextFilesFlatIterator(Self&& source) = default;
		~DirectoryTextFilesFlatIterator() = default;

		Self& operator=(Self&& rhs) = default;

		operator bool() const;
		Self& operator++();
		std::string operator*() const;

	private:
		void toNextTextFile();

	private:
		Iterator current;
		Iterator end;
	};
}

#endif //__DIR_ITERATOR_H_INCLUDED__
