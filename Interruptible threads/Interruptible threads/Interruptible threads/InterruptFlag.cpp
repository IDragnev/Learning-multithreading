#include "InterruptFlag.h"

namespace IDragnev::Multithreading
{
	void InterruptFlag::set()
	{
		flag.store(true, std::memory_order_relaxed);
		
		auto lock = LockGuard(conditionVarsMutex);
		if (conditionVariable)
		{
			conditionVariable->notify_all();
		}
		if (conditionVariableAny)
		{
			conditionVariableAny->notify_all();
		}
	}

	bool InterruptFlag::isSet() const noexcept
	{
		return flag.load(std::memory_order_relaxed);
	}

	void InterruptFlag::setConditionVariable(std::condition_variable& c)
	{
		setConditionVariable(&c);
	}

	void InterruptFlag::clearConditionVariable() 
	{
		setConditionVariable(nullptr);
	}

	void InterruptFlag::setConditionVariable(std::condition_variable* c)
	{
		auto lock = LockGuard(conditionVarsMutex);
		conditionVariable = c;
	}
}