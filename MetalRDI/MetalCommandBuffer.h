#pragma once

#include "IRDI.h"

class MTMetalCommandBuffer : public IRDICommandBuffer
{
public:
    MTMetalCommandBuffer(id<MTLCommandQueue> CommandQueue);
    
    virtual void BeginCommand(MTRDIWindowView* View, bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth) override;
    virtual void BeginCommand(MTRDITexture* ColorBuffer, MTRDITexture* DepthBuffer,
                                 bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth) override;
    
    virtual void EndCommand() override;
    
    virtual void SetShader(MTRDIShader* Shader) override;
    virtual void SetMaterial(MTRDIMaterial* Material) override;
    virtual void SetVertexBuffer(MTRDIBuffer* VertexBuffer, MTInt32 Index = 0, MTInt32 Offset = 0) override;
	virtual void SetConstantBuffer(void* ConstantData, MTInt32 Size, MTInt32 Index) override;
    virtual void SetDepthStencilState(MTRDIDepthStencilState* DepthStencilState) override;
    virtual void SetTriangleFacingWinding(MTTriangleFacingWinding Winding) override;
    virtual void SetCullMode(MTCullMode CullMode) override;
    
    virtual void SetPixelTexture(MTRDITexture* Texture, MTInt32 Index) override;
    
    virtual void DrawMesh(MTRDIMesh* Mesh, MTInt32 SubmeshIndex) override;
    
private:
    void CreateCommandEncoder(id<MTLTexture> ColorBuffer, id<MTLTexture> DepthBuffer,
                                                    bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth);
    id<MTLCommandQueue> m_CommandQueue;
    id<MTLCommandBuffer> m_CommandBuffer;
    id<MTLRenderCommandEncoder> m_CommandEncoder;
};
