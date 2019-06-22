#include "PipelinedLabirinthSolver.h"
#include "DirectoryTextFilesFlatIterator.h"
#include "UtilityFunctions.h"
#include "Ranges\Ranges.h"
#include <future>

using IDragnev::Utility::CallOnDestruction;

namespace IDragnev::Multithreading
{
	PipelinedLabirinthSolver::FileLoader::FailedToOpen::FailedToOpen(const std::string& filename) :
		std::runtime_error{ "Failed to open " + filename }
	{
	}

	//all members act like local variables and need not be copied
	PipelinedLabirinthSolver::FileLoader::FileLoader(const FileLoader&)
	{
	}

	auto PipelinedLabirinthSolver::FileLoader::operator=(const FileLoader&) -> FileLoader&
	{
		return *this;
	}

	auto PipelinedLabirinthSolver::FileLoader::operator()(const std::string& file) -> Labirinth
	{
		auto x = CallOnDestruction{ [this]() noexcept { clear(); } };
		init(file);
		parseFile();

		return std::move(result);
	}

	void PipelinedLabirinthSolver::FileLoader::init(const std::string& filename)
	{
		openFile(filename);
		result.reserve(25);
	}

	void PipelinedLabirinthSolver::FileLoader::openFile(const std::string& filename)
	{
		file.open(filename);

		if (!file.is_open())
		{
			throw FailedToOpen{ filename };
		}
	}

	void PipelinedLabirinthSolver::FileLoader::parseFile()
	{
		char buffer[255];
		
		file.clear();

		while (!file.eof() && file.good())
		{
			file.getline(buffer, 255, '\n');
			result.emplace_back(buffer);
		}
	}

	void PipelinedLabirinthSolver::FileLoader::clear() noexcept
	{
		result.clear();
		file.close();
	}

	//all members act like local variables and need not be copied
	PipelinedLabirinthSolver::PipelinedLabirinthSolver(const Self&)
	{
	}

	PipelinedLabirinthSolver& PipelinedLabirinthSolver::operator=(const Self&)
	{
		return *this;
	}

	auto PipelinedLabirinthSolver::operator()(const std::string& path) -> Result
	{
		using Barrier = std::future<void>;

		auto x = CallOnDestruction{ [this]() noexcept { clear(); } };
		Barrier solver;
		Barrier loader;

		try
		{
			solver = std::async(std::launch::async, [this] { solveLabirinths(); });
			loader = std::async(std::launch::async, [this] { loadFiles(); });
			scanForTextFiles(path);

			loader.wait();
			solver.wait();
		}
		catch (...)
		{
			abort.store(true);
			throw;
		}

		return std::move(result);
	}

	void PipelinedLabirinthSolver::clear() noexcept
	{
		result.clear();
		labirinths.tryToClear();
		files.tryToClear();
		abort.store(false);
	}

	void PipelinedLabirinthSolver::loadFiles()
	{
		while (!abort.load())
		{
			if (auto file = files.tryToExtractFront(); 
				file != nullptr)
			{
				if(!isSentinel(file))
				{		
					load(std::move(file));
				}
				else
				{
					break;
				}
			}
			else
			{
				std::this_thread::yield();
			}
		}

		insertSentinelLabirinth();
	}

	void PipelinedLabirinthSolver::load(std::unique_ptr<std::optional<std::string>> file)
	{
		const auto& filename = file->value();
		try
		{
			labirinths.insertBack(FileLoader{}(filename));
		}
		catch (...)
		{
			std::cerr << "Failed to load " << filename << "\n";
		}
	}

	void PipelinedLabirinthSolver::insertSentinelLabirinth()
	{
		labirinths.insertBack(std::nullopt);
	}

	void PipelinedLabirinthSolver::solveLabirinths()
	{
		while(!abort.load())
		{
			if (auto lab = labirinths.tryToExtractFront(); 
				lab != nullptr)
			{
				if (!isSentinel(lab))
				{
					solve(std::move(lab));
				}
				else
				{
					break;
				}
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}

	void PipelinedLabirinthSolver::solve(std::unique_ptr<std::optional<Labirinth>> lab)
	{
		auto solver = LabirinthSolver{};

		try
		{
			const auto& labirinth = lab->value();
			result.push_back(solver(std::cbegin(labirinth), std::cend(labirinth)));
		}
		catch (std::bad_alloc&)
		{ 
			std::cerr << "Error while sloving a labirinth. No memory available\n";
		}
	}

	void PipelinedLabirinthSolver::scanForTextFiles(const std::string& path)
	{
		using IDragnev::Ranges::forEach;
		using Iterator = DirectoryTextFilesFlatIterator;

		auto it = Iterator{ path };

		forEach(it, [this](auto filename) 
		{ 
			files.insertBack(std::move(filename));
		});

		insertSentinelFile();
	}

	void PipelinedLabirinthSolver::insertSentinelFile()
	{
		files.insertBack(std::nullopt);
	}
}