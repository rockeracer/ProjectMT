#pragma once

class MTRenderCommandBuffer
{
public:
	virtual ~MTRenderCommandBuffer() {}

	MTRDIWindowView* View = nullptr;
};
