#include "VulkanCommandBuffer.h"
#include "VulkanResource.h"

MTVulkanCommandBuffer::MTVulkanCommandBuffer(VkDevice Device, VkCommandPool CommandPool, MTUInt32 RenderFrameCount, VkQueue GraphicsQueue, MTVulkanBuffer* ConstantBuffer)
	: m_Device(Device), m_GraphicsQueue(GraphicsQueue), m_ConstantBuffer(ConstantBuffer)
{
	m_CommandBuffers.Resize(RenderFrameCount);

	VkCommandBufferAllocateInfo AllocInfo = {};
	AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocInfo.commandPool = CommandPool;
	AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	AllocInfo.commandBufferCount = RenderFrameCount;

	if (vkAllocateCommandBuffers(Device, &AllocInfo, m_CommandBuffers.Data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	VkSemaphoreCreateInfo SemaphoreInfo = {};
	SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo FenceInfo = {};
	FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	m_SyncObjects.Resize(RenderFrameCount);
	for (MTUInt32 i = 0; i < RenderFrameCount; ++i)
	{
		if (vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &m_SyncObjects[i].ImageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &m_SyncObjects[i].RenderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(Device, &FenceInfo, nullptr, &m_SyncObjects[i].FrameFence) != VK_SUCCESS)
		{

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

MTVulkanCommandBuffer::~MTVulkanCommandBuffer()
{
	for (MTVulkanCommandBuffer::SyncObject SyncObject : m_SyncObjects)
	{
		vkDestroySemaphore(m_Device, SyncObject.RenderFinishedSemaphore, nullptr);
		vkDestroySemaphore(m_Device, SyncObject.ImageAvailableSemaphore, nullptr);
		vkDestroyFence(m_Device, SyncObject.FrameFence, nullptr);
	}
}

void MTVulkanCommandBuffer::BeginCommand(MTRDIWindowView* View, bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth)
{
	MTVulkanWindowView* VulkanView = dynamic_cast<MTVulkanWindowView*>(View);
	if (VulkanView == nullptr)
	{
		throw std::runtime_error("This is not vulkan view!");
	}

	vkWaitForFences(m_Device, 1, &m_SyncObjects[m_CurrentFrame].FrameFence, VK_TRUE, MTMath::NumMax<MTUInt64>());
	vkResetFences(m_Device, 1, &m_SyncObjects[m_CurrentFrame].FrameFence);

	MTUInt32 ImageIndex = VulkanView->BeginRender(m_Device, m_SyncObjects[m_CurrentFrame].ImageAvailableSemaphore);

	VkCommandBufferBeginInfo BeginInfo = {};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(m_CommandBuffers[m_CurrentFrame], &BeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo RenderPassInfo = {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassInfo.renderPass = VulkanView->SwapChain.RenderPass;
	RenderPassInfo.framebuffer = VulkanView->SwapChain.FrameDatas[ImageIndex].Framebuffer;
	RenderPassInfo.renderArea.offset = { 0, 0 };
	RenderPassInfo.renderArea.extent = VulkanView->SwapChain.Extent;

	VkClearValue VulkanClear[2];
	VulkanClear[0].color = { ClearColor[0], ClearColor[1], ClearColor[2], ClearColor[3] };
	VulkanClear[1].depthStencil = {1.0f, 0};
	
	RenderPassInfo.clearValueCount = 2;
	RenderPassInfo.pClearValues = VulkanClear;

	vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentFrame], &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport Viewport = {};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)VulkanView->SwapChain.Extent.width;
	Viewport.height = (float)VulkanView->SwapChain.Extent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_CommandBuffers[m_CurrentFrame], 0, 1, &Viewport);

	VkRect2D Scissor = {};
	Scissor.offset = { 0, 0 };
	Scissor.extent = VulkanView->SwapChain.Extent;
	vkCmdSetScissor(m_CommandBuffers[m_CurrentFrame], 0, 1, &Scissor);
}

void MTVulkanCommandBuffer::BeginCommand(MTRDITexture* ColorBuffer, MTRDITexture* DepthBuffer,
                     bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth)
{
}

void MTVulkanCommandBuffer::EndCommand(MTRDIWindowView* View)
{
	vkCmdEndRenderPass(m_CommandBuffers[m_CurrentFrame]);

	if (vkEndCommandBuffer(m_CommandBuffers[m_CurrentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore WaitSemaphores[] = { m_SyncObjects[m_CurrentFrame].ImageAvailableSemaphore };
	VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = WaitSemaphores;
	SubmitInfo.pWaitDstStageMask = WaitStages;

	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

	VkSemaphore SignalSemaphores[] = { m_SyncObjects[m_CurrentFrame].RenderFinishedSemaphore };
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &SubmitInfo, m_SyncObjects[m_CurrentFrame].FrameFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	MTVulkanWindowView* VulkanView = dynamic_cast<MTVulkanWindowView*>(View);
	if (VulkanView == nullptr)
	{
		throw std::runtime_error("This is not vulkan view!");
	}
	VulkanView->EndRender(m_SyncObjects[m_CurrentFrame].RenderFinishedSemaphore);

	m_CurrentFrame = (m_CurrentFrame + 1) % m_CommandBuffers.Num();
}

void MTVulkanCommandBuffer::SetShader(MTRDIShader* Shader)
{
}

void MTVulkanCommandBuffer::SetMaterial(MTRDIMaterial* Material)
{
	MTVulkanMaterial* VulkanMaterial = dynamic_cast<MTVulkanMaterial*>(Material);
	if (VulkanMaterial)
	{
		if (VulkanMaterial->GetPipeline())
		{
			vkCmdBindPipeline(m_CommandBuffers[m_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanMaterial->GetPipeline());
		}

		VkDescriptorSet DescriptorSet = VulkanMaterial->GetDescriptorSet(m_CurrentFrame);
		vkCmdBindDescriptorSets(m_CommandBuffers[m_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanMaterial->GetPipelineLayout(), 0, 1, &DescriptorSet, 0, nullptr);
	}
}

void MTVulkanCommandBuffer::SetVertexBuffer(MTRDIBuffer* VertexBuffer, MTInt32 Index, MTInt32 Offset)
{
	MTVulkanBuffer* VulkanBuffer = dynamic_cast<MTVulkanBuffer*>(VertexBuffer);
	if (VulkanBuffer)
	{
		VkBuffer VertexBuffers[] = { VulkanBuffer->GetBuffer(0) };
		VkDeviceSize Offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_CommandBuffers[m_CurrentFrame], 0, 1, VertexBuffers, Offsets);
	}
}

void MTVulkanCommandBuffer::SetConstantBuffer(void* ConstantData, MTInt32 Size, MTInt32 Index)
{
	m_ConstantBuffer->FillBuffer(m_CurrentFrame, ConstantData);
}

void MTVulkanCommandBuffer::SetDepthStencilState(MTRDIDepthStencilState* DepthStencilState)
{
}

void MTVulkanCommandBuffer::SetTriangleFacingWinding(MTTriangleFacingWinding Winding)
{
}

void MTVulkanCommandBuffer::SetCullMode(MTCullMode CullMode)
{

}

void MTVulkanCommandBuffer::SetPixelTexture(MTRDITexture* Texture, MTInt32 Index)
{
}

void MTVulkanCommandBuffer::DrawIndexed(MTRDIBuffer* IndexBuffer)
{
	MTVulkanBuffer* VulkanIndexBuffer = dynamic_cast<MTVulkanBuffer*>(IndexBuffer);
	if (VulkanIndexBuffer)
	{
		VkIndexType IndexType;

		MTUInt32 IndexStride = VulkanIndexBuffer->GetStride();
		switch (IndexStride)
		{
		case 2: IndexType = VK_INDEX_TYPE_UINT16;	break;
		case 4: IndexType = VK_INDEX_TYPE_UINT32;	break;
		default:
			throw std::runtime_error("Invalid index size!");
			break;
		}


		vkCmdBindIndexBuffer(m_CommandBuffers[m_CurrentFrame], VulkanIndexBuffer->GetBuffer(0), 0, IndexType);

		vkCmdDrawIndexed(m_CommandBuffers[m_CurrentFrame], VulkanIndexBuffer->GetElementCount(), 1, 0, 0, 0);
	}
}
