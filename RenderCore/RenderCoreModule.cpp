#include "RenderCoreModule.h"
#include "RenderThread.h"
#include "RenderCommandBuffer.h"

#include "Sample/RenderSample.h"
MTRenderSample g_RenderSample;

MTRenderCoreModule::MTRenderCoreModule()
{
	m_RDI = RDI::GetRDI();
    
	m_CommandBuffer.Resize(MTRenderVar::RENDER_FRAME_COUNT);
	for (MTInt32 i = 0; i < MTRenderVar::RENDER_FRAME_COUNT; ++i)
	{
		m_CommandBuffer[i] = new MTRenderCommandBuffer;
	}
    
    m_RenderThread = new MTRenderThread(this, MTRenderVar::RENDER_FRAME_COUNT);
    
    
    g_RenderSample.Init(m_RDI);
}

MTRenderCoreModule::~MTRenderCoreModule()
{

}

void MTRenderCoreModule::Init()
{
    
}

void MTRenderCoreModule::Destroy()
{
}

MTRDIWindowView* MTRenderCoreModule::CreateWindowView(void* Handle, MTUInt32 Width, MTUInt32 Height)
{
    if (m_RDI)
    {
        return m_RDI->CreateWindowView(Handle, Width, Height, MTRenderVar::BACK_BUFFER_COUNT);
    }

    return nullptr;
}

void* MTRenderCoreModule::InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc)
{
    if (m_RDI)
    {
        return m_RDI->InitGUIRender(FontTextureDesc, VertexDesc, MTRenderVar::RENDER_FRAME_COUNT);
    }
    
    return nullptr;
}

void MTRenderCoreModule::ShutdownGUIRender()
{
    if (m_RDI)
    {
        return m_RDI->ShutdownGUIRender();
    }
}

void MTRenderCoreModule::BeginRender(const MTRenderCommandBuffer* CommandBuffer)
{
    if (m_RDI)
    {
        return m_RDI->BeginRender();
    }
}

void MTRenderCoreModule::RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData)
{
    if (m_RDI)
    {
        return m_RDI->RenderGUI(View, DrawData);
    }
}

void MTRenderCoreModule::EndRender(MTRDIWindowView* View)
{
    if (m_RDI)
    {
        return m_RDI->EndRender(View);
    }
}

void MTRenderCoreModule::RenderNewFrame(MTInt32 RenderFrame)
{
    const MTRenderCommandBuffer* CommandBuffer = m_CommandBuffer[RenderFrame % MTRenderVar::RENDER_FRAME_COUNT];

	if (CommandBuffer)
	{
		BeginRender(CommandBuffer);

		g_RenderSample.Render(CommandBuffer->View);

		OnRenderGUI(RenderFrame);

		EndRender(CommandBuffer->View);
	}
}

void MTRenderCoreModule::Render(MTInt32 Frame, MTRDIWindowView* View)
{
	MTRenderCommandBuffer* CommandBuffer = m_CommandBuffer[m_RenderThread->GetFrame() % MTRenderVar::RENDER_FRAME_COUNT];

	if (CommandBuffer)
	{
		CommandBuffer->View = View;

		if (m_RDI)
		{
			m_RDI->CommitRenderCommandBuffer();
		}
	}

	m_RenderThread->NewFrame();
}

void MTRenderCoreModule::ExitRender()
{
    if (m_RenderThread)
    {
        delete m_RenderThread;
        m_RenderThread = nullptr;
    }

	g_RenderSample.Destroy();

	for (MTRenderCommandBuffer* CommandBuffer : m_CommandBuffer)
	{
		if (CommandBuffer)
		{
			delete CommandBuffer;
		}
	}
	m_CommandBuffer.Clear();
}

void MTRenderCoreModule::Wait()
{
	if (m_RenderThread)
	{
		m_RenderThread->Wait();
	}
}

void MTRenderCoreModule::WaitForRenderCompleted()
{
    if (m_RDI)
    {
        return m_RDI->WaitForRenderCompleted();
    }
}
