#include "YXLMutexObejct.h"

namespace YXL
{

	MutexObject::MutexObject()
	{
#ifdef WIN32
		hMutexObject = CreateMutex(NULL, 0, NULL);
#else
		pthread_mutex_init(&mMutex, NULL);
#endif
	}
	MutexObject::~MutexObject()
	{
#ifdef WIN32
		CloseHandle(hMutexObject);
#else
		pthread_mutex_destroy(&mMutex);
#endif
	}
	void MutexObject::lock()
	{
#ifdef WIN32
		WaitForSingleObject(hMutexObject, INFINITE);
#else
		pthread_mutex_lock(&mMutex);
#endif
	}
	void MutexObject::unlock()
	{
#ifdef WIN32
		ReleaseMutex(hMutexObject);
#else
		pthread_mutex_unlock(&mMutex);
#endif
	}
}