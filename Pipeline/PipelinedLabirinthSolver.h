#ifndef __PIPELENED_LAB_SOLVER_H_INCLUDED__
#define __PIPELENED_LAB_SOLVER_H_INCLUDED__

#include "ThreadSafeQueue.h"
#include "LabirinthSolver.h"
#include <fstream>
#include <optional>
#include <atomic>

namespace IDragnev::Multithreading
{
	class PipelinedLabirinthSolver
	{
	private:
		using Self = PipelinedLabirinthSolver;
		using Labirinth = std::vector<std::string>;

	public:
		using Result = std::vector<LabirinthSolver::Result>;

		PipelinedLabirinthSolver() = default;
		explicit PipelinedLabirinthSolver(const Self& source);
		~PipelinedLabirinthSolver() = default;

		Self& operator=(const Self& rhs);

		Result operator()(const std::string& path);

	private:
		void scanForTextFiles(const std::string& path);
		void loadFiles();
		void solveLabirinths();
		void clear() noexcept;

		void load(std::unique_ptr<std::optional<std::string>> filename);
		void solve(std::unique_ptr<std::optional<Labirinth>> lab);

		void insertSentinelFile();
		void insertSentinelLabirinth();

		template <typename Optional>
		inline static bool isSentinel(const Optional& x) noexcept { return !x->has_value(); }

	private:
		Result result;
		ThreadSafeQueue<std::optional<Labirinth>> labirinths;
		ThreadSafeQueue<std::optional<std::string>> files;
		std::atomic<bool> abort = false;
	};
}

#endif //__PIPELENED_LAB_SOLVER_H_INCLUDED__