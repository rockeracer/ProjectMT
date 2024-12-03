#pragma once

#include <mutex>

class MTSemaphore
{
public:
	CORE_API MTSemaphore(MTUInt32 uiCount);
	CORE_API void Wait();

	template<typename R, typename P>
	CORE_API bool Wait(const std::chrono::duration<R, P>& crRelTime);
	CORE_API void Signal();

private:
	MTUInt32 m_Count;
	MTMutex m_Mutex;
	MTConditionVariable m_Condition;
};
