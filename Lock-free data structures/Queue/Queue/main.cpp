#include "LockFreeQueue.h"

using IDragnev::Multithreading::LockFreeQueue;

class X 
{
public:
	X(int x = 0) : x(x) { }

private:
	int x;
};

int main()
{
	X x;
	LockFreeQueue<X> queue;

	queue.enqueue(X{});
	queue.enqueue(x);
	queue.emplace(1);

	auto result = queue.extractFront();
}
