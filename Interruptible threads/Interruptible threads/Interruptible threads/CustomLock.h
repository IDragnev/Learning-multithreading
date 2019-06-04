
namespace IDragnev::Multithreading
{
	template <typename Lockable>
	void InterruptFlag::wait(std::condition_variable_any& condition, Lockable& lock)
	{
		auto customLock = CustomLock(this, condition, lock);
		checkForInterruption();
		condition.wait(customLock);
		checkForInterruption();
	}
	
	template <typename Lock>
	InterruptFlag::CustomLock<Lock>::CustomLock(InterruptFlag* flag,
		                                        std::condition_variable_any& condition,
		                                        Lock& lockable) :
		flag(flag),
		lockable(lockable)
	{
		flag->conditionVarsMutex.lock();
		flag->conditionVariableAny = &condition;
	}

	template <typename Lock>
	InterruptFlag::CustomLock<Lock>::~CustomLock()
	{
		flag->conditionVariableAny = nullptr;
		flag->conditionVarsMutex.unlock();
	}

	template <typename Lock>
	void InterruptFlag::CustomLock<Lock>::unlock()
	{
		lockable.unlock();
		flag->conditionVarsMutex.unlock();
	}

	template <typename Lock>
	void InterruptFlag::CustomLock<Lock>::lock()
	{
		std::lock(flag->conditionVasMutex, lockable);
	}
}