#include <iostream>
#include "PipelinedLabirinthSolver.h"

using Solver = IDragnev::Multithreading::PipelinedLabirinthSolver;

auto print = [](auto solutions)
{
	auto i = 1u;
	for (const auto& s : solutions)
	{
		std::cout << "labirinth " << i++ << " paths:\n";
		
		for (const auto& path : s)
		{
			std::cout << path.c_str() << " ";
		}

		std::cout << "\n\n";
	}
};

int main()
{
	print(Solver{}("Labirinths"));	
}

