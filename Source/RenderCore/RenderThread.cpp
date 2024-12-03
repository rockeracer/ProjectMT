#include "RenderThread.h"
#include "RenderCoreModule.h"

MTRenderThread::MTRenderThread(MTRenderCoreModule* RenderCoreModule, MTInt32 CommandBufferCount)
	: m_RenderCoreModule(RenderCoreModule), m_Semaphore(CommandBufferCount)
{
	m_RenderThread = MTThread(&MTRenderThread::ThreadFunction, this);
}

MTRenderThread::~MTRenderThread()
{
	{
		MTLockGuard<MTMutex> lock(m_Mutex);
		m_bRenderLoop = false;
	}
	m_RenderCondition.notify_one();
	m_RenderThread.join();
}

void MTRenderThread::Wait()
{
	m_Semaphore.Wait();
}

void MTRenderThread::NewFrame()
{
	{
		MTLockGuard<MTMutex> lock(m_Mutex);
		++m_Frame;
	}
	m_RenderCondition.notify_one();
}

void MTRenderThread::ThreadFunction()
{
	MT_LOG("Start RenderThread.\n");

	while (m_bRenderLoop)
	{
		MTUniqueLock<MTMutex> lock(m_Mutex);
		m_RenderCondition.wait(lock, [&] { return (m_bRenderLoop == false) || (m_Frame > m_RenderFrame); });
		{
			lock.unlock();

			if (m_bRenderLoop == false)
			{
				if (m_RenderCoreModule)
				{
					m_RenderCoreModule->WaitForRenderCompleted();
				}

				break;
			}

			if (m_RenderCoreModule)
			{
				m_RenderCoreModule->RenderNewFrame(m_RenderFrame);
			}

			++m_RenderFrame;

			m_Semaphore.Signal();
		}
	}

	MT_LOG("End RenderThread.\n");
}

