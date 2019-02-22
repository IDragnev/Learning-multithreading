#pragma once
#include <thread>

namespace IDragnev
{
	namespace Threads
	{
		class SmartThread
		{
		public:
			SmartThread(std::thread t) :
				thread(std::move(t))
			{
			}

			~SmartThread()
			{
				if (thread.joinable())
				{
					thread.join();
				}
			}

			SmartThread(SmartThread&&) = default;
			SmartThread(const SmartThread&) = delete;

			SmartThread& operator=(SmartThread&&) = default;
			SmartThread& operator=(const SmartThread&) = delete;

		private:
			std::thread thread;
		};
	}
}