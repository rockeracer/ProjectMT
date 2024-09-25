#pragma once

#include "imguiController.h"

struct MTGUIRenderCommandBuffer;
class MTWindow;

class MTGUIRenderer
{
public:
	MTGUIRenderer();
	~MTGUIRenderer();

	void Render(MTWindow* Window, const MTGUIInputState& InputState, float DeltaTime);

private:
	void RenderThread();

	void RenderGUICallback(MTInt32 RenderFrame);

	MTInt32 m_CommandBufferCount = 0;

	MTimguiController m_imguiController;

	MTGUIRenderCommandBuffer* m_CommnadBuffer = nullptr;

	MTInt32 m_Frame = 0;
};
