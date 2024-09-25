#pragma once

#include "IRenderCoreModule.h"

class MTRenderCommandBuffer;
class MTRenderThread;
class IRDI;

class MTRenderCoreModule : public IRenderCoreModule
{
public:
	MTRenderCoreModule();
	virtual ~MTRenderCoreModule();

	virtual void Init() override;
	virtual void Destroy() override;

	virtual MTRDIWindowView* CreateWindowView(void* Handle, MTUInt32 Width, MTUInt32 Height) override;

	virtual void Render(MTInt32 Frame, MTRDIWindowView* View) override;

	virtual void* InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc) override;

	virtual void ShutdownGUIRender() override;

	virtual void RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData) override;

	virtual void ExitRender() override;

	virtual void Wait() override;


	void RenderNewFrame(MTInt32 RenderFrame);
	void WaitForRenderCompleted();

private:
	virtual void BeginRender(const MTRenderCommandBuffer* CommandBuffer)  override;

	virtual void EndRender(MTRDIWindowView* View)  override;

	IRDI* m_RDI = nullptr;

	MTArray<MTRenderCommandBuffer*> m_CommandBuffer;

	MTRenderThread* m_RenderThread = nullptr;
};
