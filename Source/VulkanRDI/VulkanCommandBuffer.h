#pragma once

class MTVulkanBuffer;

class MTVulkanCommandBuffer : public IRDICommandBuffer
{
public:
	MTVulkanCommandBuffer(VkDevice Device, VkCommandPool CommandPool, MTUInt32 RenderFrameCount, VkQueue GraphicsQueue, MTVulkanBuffer* ConstantBuffer);
	virtual ~MTVulkanCommandBuffer();

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
	VkDevice m_Device = nullptr;
	VkQueue m_GraphicsQueue = nullptr;
	MTVulkanBuffer* m_ConstantBuffer = nullptr;

	MTArray<VkCommandBuffer> m_CommandBuffers;

	struct SyncObject
	{
		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;
		VkFence FrameFence;
	};
	MTArray<SyncObject> m_SyncObjects;

	MTUInt32 m_CurrentFrame = 0;
};
