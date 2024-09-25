#pragma once

class MTDX12GUI
{
public:
	MTDX12GUI();

	void* Init(const ComPtr<ID3D12Device>& DX12Device, const ComPtr<ID3D12GraphicsCommandList>& CommandList, MTInt32 RenderFrameCount, const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc);
    void Destroy();

	void BeginRender(MTRDIWindowView* View);
    void RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData);
    void EndRender(MTRDIWindowView* View);

private:
	void* CreateFontsTexture(const MTGUIFontTextureDesc& FontTextureDesc);

	ComPtr<ID3D12Device> m_DX12Device;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D10Blob> m_VertexShaderBlob;
	ComPtr<ID3D10Blob> m_PixelShaderBlob;
	ComPtr<ID3D12Resource> m_FontTextureResource;
	ComPtr<ID3D12DescriptorHeap> m_FontDescHeap;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	
	D3D12_CPU_DESCRIPTOR_HANDLE m_FontSrvCPUDescHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_FontSrvGPUDescHandle;

	struct FrameResources
	{
		ComPtr<ID3D12Resource> IB;
		ComPtr<ID3D12Resource> VB;
		MTInt32 VertexBufferSize = 5000;
		MTInt32 IndexBufferSize = 10000;
	};
	FrameResources* m_FrameResources = nullptr;
	UINT m_numFramesInFlight = 3;
	UINT m_FrameIndex = UINT_MAX;

	struct VERTEX_CONSTANT_BUFFER
	{
		float   mvp[4][4];
	};
};
