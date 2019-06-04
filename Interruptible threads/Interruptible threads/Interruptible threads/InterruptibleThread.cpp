#include "InterruptibleThread.h"

namespace IDragnev::Multithreading
{
	InterruptibleThread::InterruptibleThread(InterruptibleThread&& source) noexcept :
		thread(std::move(source.thread)),
		flag(source.flag)
	{
		source.flag = nullptr;
	}

	InterruptibleThread& InterruptibleThread::operator=(InterruptibleThread&& rhs) noexcept
	{
		if (this != &rhs)
		{
			auto temp = std::move(rhs);
			std::swap(thread, temp.thread);
			std::swap(flag, temp.flag);
		}

		return *this;
	}

	void InterruptibleThread::detach()
	{
		thread.detach();
		flag = nullptr;
	}

	void InterruptibleThread::join()
	{
		thread.join();
		flag = nullptr;
	}

	bool InterruptibleThread::joinable() const noexcept
	{
		return thread.joinable();
	}

	void InterruptibleThread::interrupt()
	{
		if (flag)
		{
			flag->set();
		}
	}

	void checkForInterruption()
	{
		if (interruptFlag.isSet())
		{
			throw ThreadInterrupted{};
		}
	}

	void interruptibleWait(std::condition_variable& condition,
		                   std::unique_lock<std::mutex> lock)
	{
		using Utility::CallOnDestruction;
		using namespace std::chrono_literals;

		auto clearConditionVariable = [] { interruptFlag.clearConditionVariable(); };

		checkForInterruption();
		interruptFlag.setConditionVariable(condition);
		auto x = CallOnDestruction{ clearConditionVariable };
		checkForInterruption();
		condition.wait_for(lock, 1ms);
		checkForInterruption();
	}
}