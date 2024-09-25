#pragma once

class MTRenderCoreModule;

class MTRenderThread
{
public:
    MTRenderThread(MTRenderCoreModule* RenderCoreModule, MTInt32 CommandBufferCount);
    ~MTRenderThread();
    
    void Wait();
    
    void NewFrame();

	MTInt32 GetFrame() const
	{
		return m_Frame;
	}

private:
    void ThreadFunction();
    
    MTRenderCoreModule* m_RenderCoreModule = nullptr;

    MTThread m_RenderThread;
    MTSemaphore m_Semaphore;
    MTMutex m_Mutex;
    bool m_bRenderLoop = true;
    MTConditionVariable m_RenderCondition;

    MTInt32 m_Frame = 0;
    MTInt32 m_RenderFrame = 0;
};
