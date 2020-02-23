#pragma once

#include "YXLMessageQueue.h"
#include "YXLThreadEntity.h"

namespace YXL
{
	
	class LoopThread :public ThreadEntity
	{
	public:
		LoopThread();
		virtual ~LoopThread();
		void postMessage(void *pMessage);
		virtual void dealThreadMessage(void *pMessage) = 0;
	protected:
		virtual void threadFunction();
		MessageQueue messageQueue;
	};
}