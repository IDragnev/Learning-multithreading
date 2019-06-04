#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace IDragnev::Multithreading
{
	class InterruptFlag
	{
	private:
		using LockGuard = std::lock_guard<std::mutex>;

		template <typename Lock>
		class CustomLock
		{
			CustomLock(InterruptFlag* flag,
				       std::condition_variable_any& condition,
				       Lock& lockable);
			~CustomLock();

			void lock();
			void unlock();

		private:
			InterruptFlag* flag;
			Lock& lockable;
		};

	public:
		InterruptFlag() = default;

		bool isSet() const noexcept;
		void set();

		void setConditionVariable(std::condition_variable& c);
		void clearConditionVariable();
		template <typename Lockable>
		void wait(std::condition_variable_any& c, Lockable& lock);

	private:
		void setConditionVariable(std::condition_variable* c);

	private:
		std::atomic<bool> flag = false;
		std::condition_variable* conditionVariable = nullptr;
		std::condition_variable_any* conditionVariableAny = nullptr;
		std::mutex conditionVarsMutex;
	};
}

#include "CustomLock.h"