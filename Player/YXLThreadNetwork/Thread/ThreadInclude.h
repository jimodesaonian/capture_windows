#pragma once
#include "YXLSignalObject.h"
#include "YXLMutexObejct.h"
#include "YXLThreadEntity.h"
#include "YXLLoopThread.h"

namespace YXL
{
	template<class ValueType>
	class MultipleThreadObjectTemplate
	{
	public:
		MultipleThreadObjectTemplate<ValueType>()
		{
		}
		MultipleThreadObjectTemplate<ValueType>(const ValueType&value)
		{
			*this = value;
		}
		~MultipleThreadObjectTemplate<ValueType>()
		{
		}
		ValueType& operator=(const ValueType&other)
		{
			this->value = other;
			return value;
		}
		ValueType& operator->()
		{
			return value;
		}
		void lock()
		{
			mutexObject.lock();
		}
		void unlock()
		{
			mutexObject.unlock();
		}
		void waitSignal()
		{
			signalObject.waitSignal();
		}
		void waitSignal(unsigned int millisecond)
		{
			signalObject.waitSignal(millisecond);
		}
		void sendSignal()
		{
			signalObject.sendSignal();
		}
		void emptySignal()
		{
			signalObject.emptySignal();
		}
		ValueType value;
	private:
		MutexObject mutexObject;
		SignalObject signalObject;
		
	};


}