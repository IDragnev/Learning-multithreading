#include "PipelinedLabirinthSolver.h"
#include "DirectoryTextFilesFlatIterator.h"
#include "UtilityFunctions.h"
#include "Ranges\Ranges.h"
#include "Functional\Functional.h"
#include <future>
#include "range/v3/all.hpp" 

using IDragnev::Utility::CallOnDestruction;

namespace IDragnev::Multithreading
{
	class FailedToOpen : public std::runtime_error
	{
	public:
		FailedToOpen(const std::string& filename) :
			std::runtime_error{ "Failed to open " + filename }
		{
		}
	};

	const auto openFile = [](const auto& filename) -> std::ifstream
	{
		auto file = std::ifstream{ filename };

		if (!file.is_open())
		{
			throw FailedToOpen{ filename };
		}

		return file;
	};

	const auto parseFile = [](std::ifstream file) -> std::vector<std::string>
	{
		return ranges::istream_range<std::string>{ file };
	};

	auto loadFile = Functional::compose(parseFile, openFile);

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
			labirinths.insertBack(loadFile(filename));
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