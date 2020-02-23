#pragma once

#include "YXLMutexObejct.h"
#include "YXLSignalObject.h"
#include <deque>

namespace YXL
{
	
	class MessageQueue
	{
	public:
		void pushMessage(void *msg);
		void *popWaitMessage();
		bool hasMessage();
		int getMessageQueueLength();
	private:
		std::deque<void*> messageQueue;
		MutexObject messageQueueMutex;
		SignalObject messageQueueSignal;
	};
}