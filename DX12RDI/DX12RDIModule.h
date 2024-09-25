#pragma once

#include "IRDI.h"
#include "DX12ResourceUploader.h"
#include "DX12ShaderCodeFormat.h"

class MTDX12GUI;
class IDX12UploadResource;

class MTDX12RDI : public IRDI
{
public:
	MTDX12RDI();
    virtual ~MTDX12RDI();

    virtual void Init() override;
    virtual void Destroy() override;
    
	virtual MTRDIWindowView* CreateWindowView(void* Handle, MTUInt32 Width, MTUInt32 Height, MTInt32 BackbufferCount) override;
    
    virtual void* InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc, MTInt32 RenderFrameCount) override;
    virtual void ShutdownGUIRender() override;

	virtual void CommitRenderCommandBuffer() override;
	virtual void BeginRender() override;
    virtual void RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData) override;
    virtual void EndRender(MTRDIWindowView* View) override;
    virtual void WaitForRenderCompleted() override;
    virtual MTRDIShader* CreateShaderWithSource(const MTString& Source, const MTString& VertexShaderFunction, const MTString& PixelShaderFunction) override;
    
    virtual IRDICommandBuffer* CreateCommandBuffer() override;
	virtual MTRDIBuffer* CreateVertexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 VertexSize) override;
	virtual MTRDIBuffer* CreateIndexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 IndexSize)  override;
    virtual MTRDIDepthStencilState* CreateDepthStencilState(MTCompareFunction CompareFunction, bool bDepthWrite) override;
    virtual MTRDITexture* CreateTextureFromFilePath(const MTString& FilePath) override;
	virtual MTRDITexture* CreateTextureFromMemory(MTUInt32 Width, MTUInt32 Height, MTUChar* Pixels, MTUInt32 Size) override;
	virtual MTRDIMaterial* CreateMaterial(const MTString& VertexShaderCode, const MTString& PixelShaderCode, const MTArray<MTRDITexture*> Textures) override;
    
	virtual IShaderCodeFormat* GetShaderCodeFormat() override { return &m_ShaderCodeFormat; }

private:
	void CreateDevice();
	void CreateCommandQueue();
	void CreateCommandList();
	void CreateFence();
	void CreateRootSignature();

	void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

	void ResizeView(MTRDIWindowView* View, MTUInt32 Width, MTUInt32 Height);

	void WaitForLastFrameCompleted();

	ComPtr<IDXGIFactory4> m_GIFactory;
	ComPtr<ID3D12Device> m_DX12Device;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<ID3D12RootSignature> m_RootSignature;

	MTDX12Fence m_Fence;
 	UINT64 m_FenceValue = 0;

	MTDX12GUI* m_DX12GUI = nullptr;

	MTDX12ResourceUploader m_ResourceUploader;

	std::unique_ptr<DirectX::GraphicsMemory> m_GraphicsMemory;

	MTDX12ShaderCodeFormat m_ShaderCodeFormat;
};
