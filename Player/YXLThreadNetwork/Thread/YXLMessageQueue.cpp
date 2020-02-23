#include "YXLMessageQueue.h"

namespace YXL
{

	void MessageQueue::pushMessage(void *msg)
	{
		messageQueueMutex.lock();
		messageQueue.push_back(msg);
		messageQueueMutex.unlock();
		messageQueueSignal.sendSignal();
	}
	void *MessageQueue::popWaitMessage()
	{
		bool hasValue = false;
		void *ret = NULL;
		while (!hasValue)
		{
			messageQueueMutex.lock();
			if (messageQueue.size() == 0)
			{
				messageQueueSignal.emptySignal();
			}
			else
			{
				messageQueueSignal.sendSignal();
			}
			messageQueueMutex.unlock();
			messageQueueSignal.waitSignal();

			messageQueueMutex.lock();
			if (messageQueue.size() != 0)
			{
				ret = messageQueue[0];
				messageQueue.pop_front();
				hasValue = true;
			}
			messageQueueMutex.unlock();
		}
		return ret;
	}
	bool MessageQueue::hasMessage()
	{
		bool ret;
		messageQueueMutex.lock();
		ret = messageQueue.size() > 0;
		messageQueueMutex.unlock();
		return ret;
	}

	int MessageQueue::getMessageQueueLength()
	{
		int ret;
		messageQueueMutex.lock();
		ret = (int)messageQueue.size();
		messageQueueMutex.unlock();
		return ret;
	}
}