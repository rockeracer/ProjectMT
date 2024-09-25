#pragma once

class MTDX12CommandBuffer : public IRDICommandBuffer
{
public:
	MTDX12CommandBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, DirectX::GraphicsMemory* GraphicsMemory);

	virtual void BeginCommand(MTRDIWindowView* View, bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth) override;
	virtual void BeginCommand(MTRDITexture* ColorBuffer, MTRDITexture* DepthBuffer,
		bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth) override;

	virtual void EndCommand(MTRDIWindowView* View) override;

	virtual void SetShader(MTRDIShader* Shader) override;
	virtual void SetMaterial(MTRDIMaterial* Material) override;
	virtual void SetVertexBuffer(MTRDIBuffer* VertexBuffer, MTInt32 Index = 0, MTInt32 Offset = 0) override;
	virtual void SetConstantBuffer(void* ConstantData, MTInt32 Size, MTInt32 Index) override;
	virtual void SetDepthStencilState(MTRDIDepthStencilState* DepthStencilState) override;
	virtual void SetTriangleFacingWinding(MTTriangleFacingWinding Winding) override;
	virtual void SetCullMode(MTCullMode CullMode) override;

	virtual void SetPixelTexture(MTRDITexture* Texture, MTInt32 Index) override;

	virtual void DrawIndexed(MTRDIBuffer* IndexBuffer) override;

private:
	ID3D12Device* m_Device = nullptr;
	ID3D12GraphicsCommandList* m_CommandList = nullptr;
	DirectX::GraphicsMemory* m_GraphicsMemory = nullptr;
};
