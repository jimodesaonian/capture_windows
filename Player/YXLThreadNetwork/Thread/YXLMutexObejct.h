#pragma once
#include "ThreadDefines.h"
namespace YXL
{
	class MutexObject
	{
	public:
		MutexObject();
		~MutexObject();
		void lock();
		void unlock();
	private:
#ifdef WIN32
		HANDLE hMutexObject;
#else
		pthread_mutex_t mMutex;
#endif
	};
}