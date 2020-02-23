#include "YXLSignalObject.h"
#ifdef WIN32
#else
#include <sys/time.h>
#endif

namespace YXL
{
	SignalObject::SignalObject()
	{
#ifdef WIN32
		hSingalObject = CreateEvent(NULL, 1, 0, NULL);
#else
   //     mSignaleObejct=PTHREAD_COND_INITIALIZER;
    //    mSignalMutex=PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_init(&mSignaleObejct,NULL);
        pthread_mutex_init(&mSignalMutex,NULL);
        bHasSignal=false;
#endif
	}

	SignalObject::~SignalObject()
	{
#ifdef WIN32
		CloseHandle(hSingalObject);
#else
        pthread_cond_destroy(&mSignaleObejct);
        pthread_mutex_destroy(&mSignalMutex);
#endif
	}
	void SignalObject::waitSignal(unsigned int millisecond)
	{
#ifdef WIN32
		WaitForSingleObject(hSingalObject, millisecond);
#else
		time_t second=(time_t)(millisecond/1000);
		long nsecond=(long)((millisecond%1000)*1000000);
		timeval currentTime;
        timespec time;
		gettimeofday(&currentTime,NULL);
		time.tv_sec=currentTime.tv_sec+second;
		time.tv_nsec=currentTime.tv_usec*1000+nsecond;
        pthread_cond_timedwait(&mSignaleObejct, &mSignalMutex,&time);
#endif
	}
	void SignalObject::waitSignal()
    {
#ifdef WIN32
		WaitForSingleObject(hSingalObject,INFINITE);
#else

        pthread_mutex_lock(&mSignalMutex);
        if(!bHasSignal)
        {
            pthread_cond_wait(&mSignaleObejct, &mSignalMutex);
        }
        pthread_mutex_unlock(&mSignalMutex);

#endif
    }
	void SignalObject::emptySignal()
	{
#ifdef WIN32
		ResetEvent(hSingalObject);
#else
     //   pthread_cond_init(&mSignaleObejct,NULL);
        pthread_mutex_lock(&mSignalMutex);
        bHasSignal=false;
        pthread_mutex_unlock(&mSignalMutex);

#endif
	}

	void SignalObject::sendSignal()
    {
#ifdef WIN32
		SetEvent(hSingalObject);
#else
        pthread_mutex_lock(&mSignalMutex);
        bHasSignal=true;
        pthread_cond_signal(&mSignaleObejct);
        pthread_mutex_unlock(&mSignalMutex);


#endif
    }
}