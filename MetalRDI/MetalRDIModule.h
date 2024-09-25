#pragma once

#include "IRenderAPI.h"

class MTMetalGUI;

@protocol MTLDevice;
@protocol MTLCommandQueue;

class MTMetalRDIModule : public IRDI
{
public:
    MTMetalRDIModule();
    virtual ~MTMetalRDIModule();
    
    virtual void Init() override;
    virtual void Destroy() override;
    
    virtual MTRDIWindowView* CreateWindowView(void* Handle, const MTGUIWindowSize& Size, MTInt32 BackbufferCount) override;
    virtual void ResizeWindowView(MTRenderWindowView* View, const MTGUIWindowSize& Size) override;
    
    virtual void* InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc, MTInt32 BackbufferCount) override;
    virtual void ShutdownGUIRender() override;
    
    virtual void BeginRender(const MTRenderCommandBuffer* CommandBuffer) override;
    virtual void RenderGUI(MTRenderWindowView* View, const MTGUIViewportDesc& Viewport, ImDrawData* DrawData) override;
    virtual void EndRender(MTRenderWindowView* View) override;
    virtual void WaitForRenderCompleted() override;
    virtual MTRDIShader* CreateShaderWithSource(const MTString& Source, const MTString& VertexShaderFunction, const MTString& PixelShaderFunction) override;
    
    virtual IRDICommandBuffer* CreateCommandBuffer() override;
	virtual MTRDICommandBuffer* CreateCommandBufferResource() override;
	virtual MTRDIBuffer* CreateVertexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 VertexSize) override;
	virtual MTRDIBuffer* CreateIndexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 IndexSize) override;
	virtual MTRDIBuffer* CreateConstantBuffer(MTInt32 BufferSize, MTInt32 BackbufferCount) override;
    virtual MTRDIDepthStencilState* CreateDepthStencilState(MTCompareFunction CompareFunction, bool bDepthWrite) override;
    virtual MTRDITexture* CreateTextureFromFilePath(const MTString& FilePath) override;
    
private:
    void ResizeView(MTRenderWindowView* View, const MTGUIWindowSize& Size);

	MTRDIBuffer* CreateBuffer(const void* Data, MTInt32 Size);
    
    id<MTLDevice> m_MetalDevice;
    id<MTLCommandQueue> m_MetalCommandQueue;
    
    MTMetalGUI* m_MetalGUI = nullptr;
};
