#include "VulkanAssetCommandBuffer.h"

MTVulkanAssetCommandBuffer::MTVulkanAssetCommandBuffer(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue)
{
	m_Device = Device;
	m_Queue = Queue;

	VkCommandBufferAllocateInfo AllocInfo = {};
	AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	AllocInfo.commandPool = CommandPool;
	AllocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(Device, &AllocInfo, &m_CommandBuffer);

	VkFenceCreateInfo FenceInfo = {};
	FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if (vkCreateFence(Device, &FenceInfo, nullptr, &m_Fence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create fence for asset commandbuffer!");
	}
}

MTVulkanAssetCommandBuffer::~MTVulkanAssetCommandBuffer()
{
	vkDestroyFence(m_Device, m_Fence, nullptr);
}

void MTVulkanAssetCommandBuffer::UploadBuffer(MTVulkanBuffer* StagingBuffer, VkBuffer DestBuffer)
{
	MTVulkanBufferUploadCommand* Command = new MTVulkanBufferUploadCommand(StagingBuffer, DestBuffer);

	m_AssetCommands.Add(Command);
}

void MTVulkanAssetCommandBuffer::UploadDepthBuffer(VkImage Image, VkFormat Format)
{
	MTVulkanTextureUploadCommand* Command = new MTVulkanTextureUploadCommand;
	Command->UploadDepthBuffer(Image, Format);

	m_AssetCommands.Add(Command);
}

void MTVulkanAssetCommandBuffer::UploadTexture(MTVulkanBuffer* StagingBuffer, VkImage Image, VkFormat Format, MTUInt32 Width, MTUInt32 Height)
{
	MTVulkanTextureUploadCommand* Command = new MTVulkanTextureUploadCommand;
	Command->UploadTexture(StagingBuffer, Image, Format, Width, Height);

	m_AssetCommands.Add(Command);
}

void MTVulkanAssetCommandBuffer::CommitCommand()
{
	if (m_AssetCommands.IsEmpty())
	{
		return;
	}

	VkCommandBufferBeginInfo BeginInfo = {};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(m_CommandBuffer, &BeginInfo);
	{
		for (MTVulkanAssetCommandInterface* AssetCommand : m_AssetCommands)
		{
			if (AssetCommand)
			{
				AssetCommand->Commit(m_CommandBuffer);
			}
		}
	}
	vkEndCommandBuffer(m_CommandBuffer);

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &m_CommandBuffer;

	vkQueueSubmit(m_Queue, 1, &SubmitInfo, m_Fence);

	vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, MTMath::NumMax<MTUInt64>());
	vkResetFences(m_Device, 1, &m_Fence);

	ClearCommand();
}

void MTVulkanAssetCommandBuffer::ClearCommand()
{
	for (MTVulkanAssetCommandInterface* AssetCommand : m_AssetCommands)
	{
		if (AssetCommand)
		{
			delete AssetCommand;
		}
	}
	m_AssetCommands.Clear();
}
