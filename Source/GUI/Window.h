#pragma once

#include "GUIRender.h"

struct SDL_Window;
class MTRDIWindowView;

class MTWindow
{
public:
	MTWindow();
	virtual ~MTWindow();

	void GetWindowSize(MTUInt32& outWidth, MTUInt32& outHeight);

	void GetDisplaySize(MTUInt32& outWidth, MTUInt32& outHeight);

	MTRDIWindowView* GetRenderView() const;

	bool HasMouseFocus() const;

	MTUInt32 GetWindowID() const;

	void Resize(MTInt32 Width, MTInt32 Height);

private:
	SDL_Window* m_Window = nullptr;
	MTRDIWindowView* m_RenderView = nullptr;
};
