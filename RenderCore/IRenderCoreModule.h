#pragma once

#include "IModule.h"

class MTRDIWindowView;
struct MTGUIFontTextureDesc;
struct MTGUIVertexDesc;
struct ImDrawData;
class MTRenderCommandBuffer;

class IRenderCoreModule : public IModule
{
public:
	IRenderCoreModule()             {}
	virtual ~IRenderCoreModule()    {}

	virtual MTRDIWindowView* CreateWindowView(void* Handle, MTUInt32 Width, MTUInt32 Height) = 0;

	virtual void Render(MTInt32 Frame, MTRDIWindowView* View) = 0;

	virtual void* InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc) = 0;

	virtual void ShutdownGUIRender() = 0;

	virtual void RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData) = 0;

	virtual void ExitRender() = 0;

	virtual void Wait() = 0;

	MTFunction<void(MTInt32)> OnRenderGUI;

private:
	virtual void BeginRender(const MTRenderCommandBuffer* CommandBuffer) = 0;
	virtual void EndRender(MTRDIWindowView* View) = 0;
};
