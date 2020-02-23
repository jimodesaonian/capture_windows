#include "YXLLoopThread.h"

namespace YXL
{
	LoopThread::LoopThread()
	{
	}

	LoopThread::~LoopThread()
	{
	}

	void LoopThread::postMessage(void *pMessage)
	{
		messageQueue.pushMessage(pMessage);
	}

	void LoopThread::threadFunction()
	{
		void *pMessage = NULL;
		while (true)
		{
			pMessage = messageQueue.popWaitMessage();
			if (pMessage == NULL)
			{
				break;
			}
			dealThreadMessage(pMessage);
		}
	}
}