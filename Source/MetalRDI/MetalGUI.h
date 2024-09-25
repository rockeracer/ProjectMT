#pragma once

struct ImDrawData;
struct MTGUIFontTextureDesc;
struct MTGUIVertexDesc;
struct MTGUIViewportDesc;

struct MTRenderWindowView;



class MTMetalGUI
{
public:
    MTMetalGUI(id<MTLDevice> MetalDevice, id<MTLCommandQueue> MetalCommandQueue);
    
    void* Init(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc);
    void Destroy();
    
    void BeginRender(MTRenderWindowView* View);
    void RenderGUI(MTRenderWindowView* View, const MTGUIViewportDesc& Viewport, ImDrawData* DrawData);
    void EndRender(MTRenderWindowView* View);
    void WaitForRenderCompleted();
    
private:
    id<MTLBuffer> DequeueReusableBuffer(NSUInteger size);
    void EnqueueReusableBuffer(id<MTLBuffer> buffer);
    
    id<MTLDevice> m_MetalDevice;
    id<MTLCommandQueue> m_MetalCommandQueue;
    
    id<MTLTexture> m_MetalFontTexture;
    id<MTLSamplerState> m_MetalSamplerState;
    id<MTLRenderPipelineState> m_MetalRenderPipelineState;
    NSMutableArray<id<MTLBuffer>>* m_MtlBufferPool;
    
    id<MTLCommandBuffer> m_PresentCommandBuffer;
};
