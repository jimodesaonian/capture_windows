#include "YXLThreadEntity.h"
#include <assert.h>


namespace YXL
{
	ThreadEntity::ThreadEntity()
	{
#ifdef WIN32
		hThreadHandle=NULL;
#else
		threadStopSignal.sendSignal();
#endif
		bIsThreadRunning = false;
	}

	ThreadEntity::~ThreadEntity()
	{
	}
	void ThreadEntity::start()
	{
		if (!bIsThreadRunning)
		{
			
			bIsThreadRunning = true;
#ifdef WIN32
			hThreadHandle = CreateThread(NULL, 0, startThreadFunction, this, 0, NULL);
#else
			threadStopSignal.emptySignal();
			pthread_create(&threadID, NULL, startThreadFunction, this);
#endif
		}
	}

	bool ThreadEntity::isThreadRunning()
	{
		return bIsThreadRunning;
	}

	void ThreadEntity::waitThreadStop(unsigned int milliseconds)
	{
#ifdef WIN32
		WaitForSingleObject(hThreadHandle, milliseconds);
#else
		threadStopSignal.waitSignal(milliseconds);
#endif
	}
	void ThreadEntity::waitThreadStop()
	{
#ifdef WIN32
		WaitForSingleObject(hThreadHandle, INFINITE);
#else
		threadStopSignal.waitSignal();
#endif
		
	}

#ifdef WIN32
	DWORD WINAPI ThreadEntity::startThreadFunction(LPVOID pParam)
#else
	void* ThreadEntity::startThreadFunction(void*pParam)
#endif
	{
		ThreadEntity *pThis = (ThreadEntity*)pParam;
		pThis->threadFunction();
		pThis->bIsThreadRunning = false;
		
		
#ifdef WIN32
		return 0;
#else
		pThis->threadStopSignal.sendSignal();
		return NULL;
#endif
	}


#ifdef WIN32
	DWORD WINAPI ThreadEntity::singleThreadFunction(LPVOID pParams)
#else
	void* ThreadEntity::singleThreadFunction(void*pParams)
#endif
	{
		void*pParam = ((void**)pParams)[0];
		void(*threadFunction)(void*param) = (void(*)(void*))((void**)pParams)[1];
		delete[]((void**)pParams);
		threadFunction(pParam);
#ifdef WIN32
		return 0;
#else
		return NULL;
#endif
	}

	void ThreadEntity::sleep(unsigned int millisecond)
	{
#ifdef _WIN32
		Sleep(millisecond);
#else
		::usleep(millisecond*1000);
#endif
	}

	void ThreadEntity::runThread(void(*threadFunction)(void*param), void *pParam)
	{
		void**pParams = new void*[2];
		pParams[0] = pParam;
		pParams[1] = (void*)threadFunction;

#ifdef WIN32
		CreateThread(NULL, 0, singleThreadFunction, pParams, 0, NULL);
#else
		pthread_t threadID=0;
		pthread_create(&threadID, NULL, singleThreadFunction, pParams);
#endif
	}

	struct UnnamedThreadParam
	{
		std::function<void(void*param)> function;
		void *pParam;
	};

	static void unnamedThreadFunction(void*pParam)
	{
		UnnamedThreadParam *pUnnamedThreadParam = (UnnamedThreadParam *)pParam;
		std::function<void(void*param)> function = pUnnamedThreadParam->function;
		void*pUnamedThreadVoidParam = pUnnamedThreadParam->pParam;
		delete pUnnamedThreadParam;
		function(pUnamedThreadVoidParam);
	}

	void ThreadEntity::runThread(std::function<void(void*param)> function, void*pParam)
	{
		UnnamedThreadParam *pUnnamedThreadParam = new UnnamedThreadParam;
		pUnnamedThreadParam->function = function;
		pUnnamedThreadParam->pParam = pParam;
		runThread(unnamedThreadFunction, pUnnamedThreadParam);
	}
}