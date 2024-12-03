#pragma once

#include "IGUIModule.h"
#include "imguiController.h"
#include "InputState.h"

class MTWindow;
class MTGUIRenderer;

class MTGUIModule : public IGUIModule
{
public:
	virtual ~MTGUIModule() {}
	virtual void Init() override;
	virtual void Destroy() override;

	virtual MTWindow* CreateWindow() override;
	virtual void DestroyWindow(MTWindow* Window) override;

	virtual bool ProcessEvent() override;

	virtual void RenderWindow(MTWindow* Window, float DeltaTime) override;

	virtual void ExitRender() override;

private:
	bool ProcessEvent(const SDL_Event& event);

	MTGUIInputState m_InputState;

	MTGUIRenderer* m_Renderer = nullptr;

	MTArray<MTWindow*> m_Windows;

	MTModuleHandle SDL_DLLHandle;
};
