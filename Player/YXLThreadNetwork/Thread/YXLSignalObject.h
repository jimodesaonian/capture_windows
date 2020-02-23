#pragma once

#include "ThreadDefines.h"

namespace YXL
{
	class SignalObject
	{
	public:
		SignalObject();
		~SignalObject();
		void waitSignal(unsigned int millisecond);
		void waitSignal();
		void emptySignal();
		void sendSignal();
	private:
#ifdef WIN32
		HANDLE hSingalObject;
#else
        pthread_cond_t mSignaleObejct;
		pthread_mutex_t mSignalMutex;
		bool bHasSignal;

#endif
	};

}