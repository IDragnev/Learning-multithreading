#include "LockFreeStack.h"

using IDragnev::Multithreading::LockFreeStack;

struct X { };

int main()
{
	LockFreeStack<X> s;
}