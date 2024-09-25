#include "Semaphore.h"

MTSemaphore::MTSemaphore(MTUInt32 Count)
    : m_Count(Count)
{
}


void MTSemaphore::Wait()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Condition.wait(lock,[&]()->bool{ return m_Count>0; });
    --m_Count;
}
    
template< typename R,typename P >
bool MTSemaphore::Wait(const std::chrono::duration<R,P>& crRelTime)
{
    std::unique_lock< std::mutex > lock(m_Mutex);
    if (!m_Condition.wait_for(lock,crRelTime,[&]()->bool{ return m_Count>0; }))
    {
        return false;
    }
    
    --m_Count;
    return true;
}

void MTSemaphore::Signal()
{
    std::unique_lock< std::mutex > lock(m_Mutex);
    ++m_Count;
    m_Condition.notify_one();
}
