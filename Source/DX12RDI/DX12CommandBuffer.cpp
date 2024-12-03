#include "DX12CommandBuffer.h"
#include "DX12Resource.h"

MTDX12CommandBuffer::MTDX12CommandBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, DirectX::GraphicsMemory* GraphicsMemory)
	: m_Device(Device), m_CommandList(CommandList), m_GraphicsMemory(GraphicsMemory)
{

}

void MTDX12CommandBuffer::BeginCommand(MTRDIWindowView* View, bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth)
{
	MTDX12WindowView* DX12View = dynamic_cast<MTDX12WindowView*>(View);
	if (DX12View)
	{
		DX12View->BeginRender(m_CommandList, ClearColor.data_, ClearDepth);
	}
}

void MTDX12CommandBuffer::BeginCommand(MTRDITexture* ColorBuffer, MTRDITexture* DepthBuffer,
	bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth)
{
	MTDX12Texture* DX12ColorBuffer = dynamic_cast<MTDX12Texture*>(ColorBuffer);
	MTDX12Texture* DX12DepthBuffer = dynamic_cast<MTDX12Texture*>(DepthBuffer);

	if (DX12ColorBuffer && DX12DepthBuffer)
	{
	}
}

void MTDX12CommandBuffer::EndCommand(MTRDIWindowView* View)
{
}

void MTDX12CommandBuffer::SetShader(MTRDIShader* Shader)
{
	MTDX12Shader* DX12Shader = dynamic_cast<MTDX12Shader*>(Shader);
	if (DX12Shader)
	{
		m_CommandList->SetPipelineState(DX12Shader->PipelineState.Get());
	}
}

void MTDX12CommandBuffer::SetMaterial(MTRDIMaterial* Material)
{
	MTDX12Material* DX12Material = dynamic_cast<MTDX12Material*>(Material);
	if (DX12Material)
	{
		DX12Material->Commit(m_Device, m_CommandList);
	}
}

void MTDX12CommandBuffer::SetVertexBuffer(MTRDIBuffer* VertexBuffer, MTInt32 Index, MTInt32 Offset)
{
	MTDX12VertexBuffer* DX12VertexBuffer = dynamic_cast<MTDX12VertexBuffer*>(VertexBuffer);
	if (DX12VertexBuffer)
	{
		m_CommandList->IASetVertexBuffers(0, 1, DX12VertexBuffer->GetVertexBufferView());
	}
}

void MTDX12CommandBuffer::SetConstantBuffer(void* ConstantData, MTInt32 Size, MTInt32 Index)
{
	const MTInt32 Alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	const MTInt32 AlignedSize = (Size + Alignment - 1) & ~(Alignment - 1);

	DirectX::GraphicsResource ConstantBuffer = m_GraphicsMemory->Allocate(AlignedSize, Alignment);
	memcpy(ConstantBuffer.Memory(), ConstantData, Size);
	m_CommandList->SetGraphicsRootConstantBufferView(Index, ConstantBuffer.GpuAddress());
}

void MTDX12CommandBuffer::SetDepthStencilState(MTRDIDepthStencilState* DepthStencilState)
{
	MTDX12DepthStencilState* DX12DepthStencilState = dynamic_cast<MTDX12DepthStencilState*>(DepthStencilState);
	if (DX12DepthStencilState)
	{
	}
}

void MTDX12CommandBuffer::SetTriangleFacingWinding(MTTriangleFacingWinding Winding)
{
}

void MTDX12CommandBuffer::SetCullMode(MTCullMode CullMode)
{

}

void MTDX12CommandBuffer::SetPixelTexture(MTRDITexture* Texture, MTInt32 Index)
{
	MTDX12Texture* DX12Texture = dynamic_cast<MTDX12Texture*>(Texture);
	if (DX12Texture)
	{
	}
}

void MTDX12CommandBuffer::DrawIndexed(MTRDIBuffer* IndexBuffer)
{
	MTDX12IndexBuffer* DX12IndexBuffer = dynamic_cast<MTDX12IndexBuffer*>(IndexBuffer);
	if (DX12IndexBuffer)
	{
		m_CommandList->IASetIndexBuffer(DX12IndexBuffer->GetIndexBufferView());
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_CommandList->DrawIndexedInstanced(DX12IndexBuffer->GetElementCount(), 1, 0, 0, 0);
	}
}
