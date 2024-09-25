#pragma once

#include "IModule.h"
#include "RDIResource.h"

class MTRDIWindowView;
struct MTGUIFontTextureDesc;
struct MTGUIVertexDesc;
struct ImDrawData;
class IShaderCodeFormat;

class IRDICommandBuffer
{
public:
    virtual ~IRDICommandBuffer() {}
    
    virtual void BeginCommand(MTRDIWindowView* View, bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth) = 0;
    virtual void BeginCommand(MTRDITexture* ColorBuffer, MTRDITexture* DepthBuffer,
                              bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth) = 0;
    virtual void EndCommand(MTRDIWindowView* View) = 0;
    
    virtual void SetShader(MTRDIShader* Shader) = 0;
	virtual void SetMaterial(MTRDIMaterial* Material) = 0;
    virtual void SetVertexBuffer(MTRDIBuffer* VertexBuffer, MTInt32 Index = 0, MTInt32 Offset = 0) = 0;
	virtual void SetConstantBuffer(void* ConstantData, MTInt32 Size, MTInt32 Index) = 0;
    virtual void SetDepthStencilState(MTRDIDepthStencilState* DepthStencilState) = 0;
    virtual void SetTriangleFacingWinding(MTTriangleFacingWinding Winding) = 0;
    virtual void SetCullMode(MTCullMode CullMode) = 0;

	virtual void SetPixelTexture(MTRDITexture* Texture, MTInt32 Index) = 0;

	virtual void DrawIndexed(MTRDIBuffer* IndexBuffer) = 0;
};

class IRDI : public IModule
{
public:
    virtual MTRDIWindowView* CreateWindowView(void* Handle, MTUInt32 Width, MTUInt32 Height, MTInt32 BackbufferCount) = 0;
    
	virtual void* InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc, MTInt32 RenderFrameCount) = 0;
    virtual void ShutdownGUIRender() = 0;
    
	virtual void CommitRenderCommandBuffer() = 0;
    virtual void BeginRender() = 0;
    virtual void RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData) = 0;
    virtual void EndRender(MTRDIWindowView* View) = 0;
    virtual void WaitForRenderCompleted() = 0;
    virtual MTRDIShader* CreateShaderWithSource(const MTString& Source, const MTString& VertexShaderFunction, const MTString& PixelShaderFunction) = 0;
    
    virtual IRDICommandBuffer* CreateCommandBuffer() = 0;
	virtual MTRDIBuffer* CreateVertexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 VertexSize) = 0;
	virtual MTRDIBuffer* CreateIndexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 IndexSize) = 0;
    virtual MTRDIDepthStencilState* CreateDepthStencilState(MTCompareFunction CompareFunction, bool bDepthWrite) = 0;
    virtual MTRDITexture* CreateTextureFromFilePath(const MTString& FilePath) = 0;
	virtual MTRDITexture* CreateTextureFromMemory(MTUInt32 Width, MTUInt32 Height, MTUChar* Pixels, MTUInt32 Size) = 0;
	virtual MTRDIMaterial* CreateMaterial(const MTString& VertexShaderCode, const MTString& PixelShaderCode, const MTArray<MTRDITexture*> Textures) = 0;

	virtual IShaderCodeFormat* GetShaderCodeFormat() = 0;
};
