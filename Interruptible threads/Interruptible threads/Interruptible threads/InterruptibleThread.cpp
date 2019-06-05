#include "InterruptibleThread.h"
#include <assert.h>

using IDragnev::Multithreading::Detail::myInterruptFlag;

namespace IDragnev::Multithreading
{
	InterruptibleThread::InterruptibleThread(InterruptibleThread&& source) noexcept :
		thread(std::move(source.thread)),
		interruptFlag(source.interruptFlag)
	{
		source.interruptFlag = nullptr;
	}

	InterruptibleThread& InterruptibleThread::operator=(InterruptibleThread&& rhs) noexcept
	{
		if (this != &rhs)
		{
			assert(!thread.joinable());

			auto temp = std::move(rhs);
			std::swap(thread, temp.thread);
			std::swap(interruptFlag, temp.interruptFlag);
		}

		return *this;
	}

	void InterruptibleThread::detach()
	{
		thread.detach();
		interruptFlag = nullptr;
	}

	void InterruptibleThread::join()
	{
		thread.join();
		interruptFlag = nullptr;
	}

	bool InterruptibleThread::joinable() const noexcept
	{
		return thread.joinable();
	}

	void InterruptibleThread::interrupt()
	{
		if (interruptFlag)
		{
			interruptFlag->set();
		}
	}

	void checkForInterruption()
	{
		if (myInterruptFlag().isSet())
		{
			throw ThreadInterrupted{};
		}
	}

	void interruptibleWait(std::condition_variable& condition,
		                   std::unique_lock<std::mutex> lock)
	{
		using Utility::CallOnDestruction;
		using namespace std::chrono_literals;

		auto clearConditionVariable = [] { myInterruptFlag().clearConditionVariable(); };

		checkForInterruption();
		myInterruptFlag().setConditionVariable(condition);
		auto x = CallOnDestruction{ clearConditionVariable };
		checkForInterruption();
		//any interrupts here will be lost if the wait is not bounded
		condition.wait_for(lock, 1ms);
		checkForInterruption();
	}
}