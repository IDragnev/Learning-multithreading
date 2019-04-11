#include "LockFreeStack.h"

using IDragnev::Multithreading::LockFreeStack;

struct X
{
	X() = default;
	X(int x, int* y) :
		x(x), y(y)
	{
	}

	int x = 0;
	int* y = nullptr;
};

int main()
{
	X x;
	LockFreeStack<X> s;

	s.push({});
	s.push(x);
	s.emplace(1, nullptr);
}