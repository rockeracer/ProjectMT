#pragma once

#include "VulkanResource.h"

class MTVulkanBuffer;

class MTVulkanAssetCommandInterface
{
public:
	virtual ~MTVulkanAssetCommandInterface() {}
	virtual void Commit(VkCommandBuffer CommandBuffer) = 0;
};

class MTVulkanBufferUploadCommand : public MTVulkanAssetCommandInterface
{
public:
	MTVulkanBufferUploadCommand(MTVulkanBuffer* StagingBuffer, VkBuffer DestBuffer)
		: m_StagingBuffer(StagingBuffer), m_DestBuffer(DestBuffer)
	{}

	virtual ~MTVulkanBufferUploadCommand()
	{
		if (m_StagingBuffer)
		{
			delete m_StagingBuffer;
		}
	}

	virtual void Commit(VkCommandBuffer CommandBuffer) override
	{
		VkBufferCopy CopyRegion = {};
		CopyRegion.srcOffset = 0;
		CopyRegion.dstOffset = 0;
		CopyRegion.size = m_StagingBuffer->GetBufferSize();
		vkCmdCopyBuffer(CommandBuffer, m_StagingBuffer->GetBuffer(0), m_DestBuffer, 1, &CopyRegion);
	}

private:
	MTVulkanBuffer* m_StagingBuffer = nullptr;
	VkBuffer m_DestBuffer = nullptr;
};

class MTVulkanTextureUploadCommand : public MTVulkanAssetCommandInterface
{
public:
	virtual ~MTVulkanTextureUploadCommand()
	{
		if (m_StagingBuffer)
		{
			delete m_StagingBuffer;
		}
	}

	void UploadDepthBuffer(VkImage Image, VkFormat Format)
	{
		m_bDepthBuffer = true;
		m_Image = Image;
		m_Format = Format;
	}

	void UploadTexture(MTVulkanBuffer* StagingBuffer, VkImage Image, VkFormat Format, MTUInt32 Width, MTUInt32 Height)
	{
		m_bDepthBuffer = false;
		m_StagingBuffer = StagingBuffer;
		m_Image = Image;
		m_Format = Format;
		m_Width = Width;
		m_Height = Height;
	}

	virtual void Commit(VkCommandBuffer CommandBuffer) override
	{
		if (m_bDepthBuffer)
		{
			TransitionImageLayout(CommandBuffer, m_Image, m_Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
		else
		{
			TransitionImageLayout(CommandBuffer, m_Image, m_Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			{
				VkBufferImageCopy Region = {};
				Region.bufferOffset = 0;
				Region.bufferRowLength = 0;
				Region.bufferImageHeight = 0;
				Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				Region.imageSubresource.mipLevel = 0;
				Region.imageSubresource.baseArrayLayer = 0;
				Region.imageSubresource.layerCount = 1;
				Region.imageOffset = { 0, 0, 0 };
				Region.imageExtent = { m_Width, m_Height, 1 };

				vkCmdCopyBufferToImage(CommandBuffer, m_StagingBuffer->GetBuffer(0), m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
			}
			TransitionImageLayout(CommandBuffer, m_Image, m_Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}

private:
	void TransitionImageLayout(VkCommandBuffer CommandBuffer, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout)
	{
		VkImageMemoryBarrier Barrier = {};
		Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		Barrier.oldLayout = OldLayout;
		Barrier.newLayout = NewLayout;
		Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.image = Image;
		Barrier.subresourceRange.baseMipLevel = 0;
		Barrier.subresourceRange.levelCount = 1;
		Barrier.subresourceRange.baseArrayLayer = 0;
		Barrier.subresourceRange.layerCount = 1;

		if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (MTVulkanUtils::IsStencilFormat(Format))
			{
				Barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkPipelineStageFlags SrcStage;
		VkPipelineStageFlags DestStage;

		if ((OldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
		{
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			DestStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if ((OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
		{
			Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			DestStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if ((OldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))
		{
			Barrier.srcAccessMask = 0;
			Barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			DestStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			CommandBuffer,
			SrcStage, DestStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &Barrier
		);
	}

	bool m_bDepthBuffer = false;
	VkImage m_Image = nullptr;
	VkFormat m_Format;
	MTVulkanBuffer* m_StagingBuffer = nullptr;
	MTUInt32 m_Width = 0;
	MTUInt32 m_Height = 0;
};

class MTVulkanAssetCommandBuffer
{
public:
	MTVulkanAssetCommandBuffer(VkDevice Device, VkCommandPool CommandPool, VkQueue Queue);
	~MTVulkanAssetCommandBuffer();

	void UploadBuffer(MTVulkanBuffer* StagingBuffer, VkBuffer DestBuffer);
	
	void UploadDepthBuffer(VkImage Image, VkFormat Format);

	void UploadTexture(MTVulkanBuffer* StagingBuffer, VkImage Image, VkFormat Format, MTUInt32 Width, MTUInt32 Height);

	void CommitCommand();

	void ClearCommand();

private:
	VkDevice m_Device = nullptr;
	VkQueue m_Queue = nullptr;

	VkCommandBuffer m_CommandBuffer = nullptr;
	VkFence m_Fence = nullptr;

	MTArray<MTVulkanAssetCommandInterface*> m_AssetCommands;
};
