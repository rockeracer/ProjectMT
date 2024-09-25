#include "GUIRenderer.h"
#include "InputState.h"

#include "RenderCore.h"

struct MTGUIRenderCommandBuffer
{
    MTWindow* Window = nullptr;
    MTGUIInputState InputState;
    float DeltaTime = 0;
};

MTGUIRenderer::MTGUIRenderer()
{
    IRenderCoreModule* RenderCore = Core::LoadModule<IRenderCoreModule>("RenderCore");
    if (RenderCore)
    {
        m_CommandBufferCount = MTRenderVar::RENDER_FRAME_COUNT;
        
        m_imguiController.Init();
        
        m_CommnadBuffer = new MTGUIRenderCommandBuffer[m_CommandBufferCount];
        
        RenderCore->OnRenderGUI = std::bind(&MTGUIRenderer::RenderGUICallback, this, std::placeholders::_1);
    }
}

MTGUIRenderer::~MTGUIRenderer()
{
	IRenderCoreModule* RenderCore = Core::LoadModule<IRenderCoreModule>("RenderCore");
    if (RenderCore)
    {
        RenderCore->ExitRender();

		RenderCore->ShutdownGUIRender();
    }

    if (m_CommnadBuffer)
    {
        delete[] m_CommnadBuffer;
    }

    m_imguiController.Shutdown();
}

void MTGUIRenderer::Render(MTWindow* Window, const MTGUIInputState& InputState, float DeltaTime)
{
    if (Window == nullptr)
    {
        return;
    }

	IRenderCoreModule* RenderCore = Core::LoadModule<IRenderCoreModule>("RenderCore");
    if (RenderCore)
    {
		RenderCore->Wait();

		MTInt32 Index = m_Frame % m_CommandBufferCount;

		m_CommnadBuffer[Index].Window = Window;
		m_CommnadBuffer[Index].InputState = InputState;
		m_CommnadBuffer[Index].DeltaTime = DeltaTime;

		RenderCore->Render(m_Frame, Window->GetRenderView());

		++m_Frame;
    }
}

void MTGUIRenderer::RenderGUICallback(MTInt32 RenderFrame)
{
    const MTGUIRenderCommandBuffer& CommnadBuffer = m_CommnadBuffer[RenderFrame % m_CommandBufferCount];
    
    m_imguiController.InitFrame(CommnadBuffer.Window, CommnadBuffer.InputState, CommnadBuffer.DeltaTime);
    m_imguiController.ShowSampleUI();
    m_imguiController.Render(CommnadBuffer.Window->GetRenderView());
}
