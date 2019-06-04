#pragma once

#include "UtilityFunctions.h"
#include "InterruptFlag.h"
#include <thread>
#include <future>

namespace IDragnev::Multithreading
{
	InterruptFlag& myInterruptFlag()
	{
		static thread_local InterruptFlag interruptFlag;

		return interruptFlag;
	}

	class ThreadInterrupted : public std::exception { };

	class InterruptibleThread
	{
	private:
		template <typename Function>
		using EnableIfNotSelf = std::enable_if_t<!std::is_same_v<std::decay_t<Function>,
			                                                     InterruptibleThread>>;
		using FlagPromise = std::promise<InterruptFlag*>;

	public:
		InterruptibleThread() = default;
		InterruptibleThread(InterruptibleThread&& source) noexcept;
		~InterruptibleThread() = default;

		template<typename Function, typename... Args,
			     typename = EnableIfNotSelf<Function>>
		explicit InterruptibleThread(Function&& f, Args&&... args);

		InterruptibleThread& operator=(InterruptibleThread&& rhs) noexcept;

		void join();
		void detach();
		bool joinable() const noexcept;
		void interrupt();

	private:
		std::thread thread;
		InterruptFlag* flag = nullptr;
	};

	template<typename Function, typename... Args, typename>
	InterruptibleThread::InterruptibleThread(Function&& f, Args&&... args)
	{
		auto p = FlagPromise{};
		auto g = [&p, f = std::forward<Function>(f)](auto&&... args)
		{
			p.set_value(&myInterruptFlag());
			try
			{
				f(std::forward<decltype(args)>(args)...);
			}
			catch(ThreadInterrupted&) { }
		};

		thread = { g, std::forward<Args>(args)... };
		flag = p.get_future().get();
	}

	void checkForInterruption();
	void interruptibleWait(std::condition_variable& condition,
		                   std::unique_lock<std::mutex> lock);
}