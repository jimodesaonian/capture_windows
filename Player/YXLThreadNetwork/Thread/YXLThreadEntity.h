#pragma once
#include "YXLSignalObject.h"
#include "ThreadDefines.h"
#include <functional>
namespace YXL
{
	class ThreadEntity
	{
	public:
		ThreadEntity();
		virtual ~ThreadEntity();
		virtual void threadFunction() = 0;
		void start();
		bool isThreadRunning();
		void waitThreadStop(unsigned int millisecond);
		void waitThreadStop();
		static void sleep(unsigned  int millisecond);
		static void runThread(void(*threadFunction)(void*param),void *pParam);

		static void runThread(std::function< void(void*param) > function, void*pParam);
	private:
		
		bool bIsThreadRunning;
#ifdef WIN32
		HANDLE hThreadHandle;
		static DWORD WINAPI startThreadFunction(LPVOID pParam);
		static DWORD WINAPI singleThreadFunction(LPVOID pParam);
#else
		SignalObject threadStopSignal;
		static void* startThreadFunction(void*pParam);
		static void* singleThreadFunction(void*pParam);
		pthread_t  threadID;
#endif
	};
}