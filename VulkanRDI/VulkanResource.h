#pragma once

#include "RDIResource.h"
#include "VulkanRDIModule.h"

class MTVulkanWindowView : public MTRDIWindowView
{
public:
	MTVulkanWindowView(MTUInt32 Width, MTUInt32 Height) : MTRDIWindowView(Width, Height)
	{}

	virtual ~MTVulkanWindowView()
	{
		if (m_RDI)
		{
			m_RDI->DestroySwapChain(SwapChain);
			m_RDI->DestroySurface(Surface);
		}
	}

	virtual void OnResize(MTUInt32 Width, MTUInt32 Height) override
	{
		if (m_RDI)
		{
			vkDeviceWaitIdle(m_RDI->GetDevice());

			m_RDI->DestroySwapChain(SwapChain);
			SwapChain = m_RDI->CreateSwapChain(Surface, SwapChain.ImageCount, Width, Height);
		}
	}

	void Init(MTVulkanRDI* RDI, void* Handle, MTInt32 BackbufferCount, MTUInt32 Width, MTUInt32 Height)
	{
		if (RDI)
		{
			m_RDI = RDI;

			Surface = RDI->CreateSurface(Handle);
			SwapChain = RDI->CreateSwapChain(Surface, BackbufferCount, Width, Height);
		}
	}

	MTUInt32 BeginRender(VkDevice Device, VkSemaphore Semaphore)
	{
		vkAcquireNextImageKHR(Device, SwapChain.SwapChain, MTMath::NumMax<MTUInt64>(), Semaphore, VK_NULL_HANDLE, &m_ImageIndex);
		return m_ImageIndex;
	}

	void EndRender(VkSemaphore PresentWaitSemaphore)
	{
		m_PresentWaitSemaphore = PresentWaitSemaphore;
	}

	void Present(VkQueue PresentQueue)
	{
		VkPresentInfoKHR PresentInfo = {};
		PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore WaitSemaphores[] = { m_PresentWaitSemaphore };
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = WaitSemaphores;

		VkSwapchainKHR SwapChains[] = { SwapChain.SwapChain };
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = SwapChains;
		PresentInfo.pImageIndices = &m_ImageIndex;
		PresentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(PresentQueue, &PresentInfo);
	}

	VkSurfaceKHR Surface;
	MTVulkanSwapChain SwapChain;

private:
	MTVulkanRDI* m_RDI = nullptr;

	MTUInt32 m_ImageIndex = 0;

	VkSemaphore m_PresentWaitSemaphore;
};

class MTVulkanBuffer : public MTRDIBuffer
{
public:
	MTVulkanBuffer(MTUInt32 Size, MTUInt32 Stride) : MTRDIBuffer(Size, Stride)
	{}

	virtual ~MTVulkanBuffer()
	{
		for (VkBuffer Buffer : m_Buffers)
		{
			vkDestroyBuffer(m_Device, Buffer, nullptr);
		}

		for (VkDeviceMemory Memory : m_Memories)
		{
			vkFreeMemory(m_Device, Memory, nullptr);
		}
	}

	void Init(VkDevice Device, VkPhysicalDevice PhysicalDevice, MTInt32 BufferCount, MTInt32 Size,
		VkBufferUsageFlags Usage, VkMemoryPropertyFlagBits Properties)
	{
		m_Device = Device;
		m_BufferSize = Size;

		m_Buffers.Resize(BufferCount);
		m_Memories.Resize(BufferCount);

		for (MTInt32 i = 0; i < BufferCount ; ++i)
		{
			CreateBuffer(i, Device, Size, Usage);
			AllocateMemory(i, PhysicalDevice, Properties);
		}
	}

	void FillBuffer(MTInt32 Index, const void* BufferData)
	{
		void* Data = nullptr;
		vkMapMemory(m_Device, m_Memories[Index], 0, m_BufferSize, 0, &Data);
		memcpy(Data, BufferData, m_BufferSize);
		vkUnmapMemory(m_Device, m_Memories[Index]);
	}

	VkBuffer GetBuffer(MTInt32 Index) const { return m_Buffers[Index]; }

	const MTArray<VkBuffer>& GetBuffers() const { return m_Buffers;  }
	
	MTInt32 GetBufferSize() const { return m_BufferSize; }

protected:
	void CreateBuffer(MTInt32 Index, VkDevice Device, MTInt32 Size, VkBufferUsageFlags Usage)
	{
		m_Device = Device;
		m_BufferSize = Size;

		VkBufferCreateInfo BufferInfo = {};
		BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferInfo.size = Size;
		BufferInfo.usage = Usage;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_Device, &BufferInfo, nullptr, &m_Buffers[Index]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}
	}

	void AllocateMemory(MTInt32 Index, VkPhysicalDevice PhysicalDevice, VkMemoryPropertyFlagBits Properties)
	{
		VkMemoryRequirements MemRequirements;
		vkGetBufferMemoryRequirements(m_Device, m_Buffers[Index], &MemRequirements);

		VkMemoryAllocateInfo AllocInfo = {};
		AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocInfo.allocationSize = MemRequirements.size;
		AllocInfo.memoryTypeIndex = MTVulkanUtils::FindMemoryType(PhysicalDevice, MemRequirements.memoryTypeBits, Properties);

		if (vkAllocateMemory(m_Device, &AllocInfo, nullptr, &m_Memories[Index]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(m_Device, m_Buffers[Index], m_Memories[Index], 0);
	}

	VkDevice m_Device;
	MTInt32 m_BufferSize = 0;
	MTArray<VkBuffer> m_Buffers;
	MTArray<VkDeviceMemory> m_Memories;
};

class MTVulkanMaterial : public MTRDIMaterial
{
public:
	virtual ~MTVulkanMaterial()
	{
		if (m_RDI)
		{
			m_RDI->DestroyGraphicsPipeline(m_Pipeline);
			m_RDI->DestroyPipelineLayout(m_PipelineLayout);
			m_RDI->DestroyDescriptorSetLayout(m_DescriptorSetLayout);
			m_RDI->DestroyShaderModule(m_VertexShaderModule);
			m_RDI->DestroyShaderModule(m_PixelShaderModule);
		}
	}

	void Init(MTVulkanRDI* RDI, const MTString& VertexShaderCode, const MTString& PixelShaderCode, const MTArray<MTRDITexture*> Textures)
	{
		if (RDI)
		{
			m_RDI = RDI;

			m_DescriptorSetLayout = RDI->CreateDescriptorSetLayout(Textures.Num());
			m_PipelineLayout = RDI->CreatePipelineLayout(1, &m_DescriptorSetLayout);
			m_VertexShaderModule = RDI->CreateShaderModule(VertexShaderCode, shaderc_vertex_shader);
			m_PixelShaderModule = RDI->CreateShaderModule(PixelShaderCode, shaderc_fragment_shader);

			VkPipelineShaderStageCreateInfo VertShaderStageInfo = {};
			VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			VertShaderStageInfo.module = m_VertexShaderModule;
			VertShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo PixelShaderStageInfo = {};
			PixelShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			PixelShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			PixelShaderStageInfo.module = m_PixelShaderModule;
			PixelShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo ShaderStages[] = { VertShaderStageInfo, PixelShaderStageInfo };

			m_Pipeline = RDI->CreateGraphicsPipeline(ShaderStages, m_PipelineLayout);

			RDI->AllocateDescriptorSets(m_DescriptorSetLayout, Textures, m_DescriptorSets);
		}
	}

	VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

	VkPipeline GetPipeline() const { return m_Pipeline; }

	VkDescriptorSet GetDescriptorSet(MTInt32 Index) const { return m_DescriptorSets[Index]; }

private:
	MTVulkanRDI* m_RDI = nullptr;

	VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
	VkPipelineLayout m_PipelineLayout = nullptr;
	VkShaderModule m_VertexShaderModule = nullptr;
	VkShaderModule m_PixelShaderModule = nullptr;
	VkPipeline m_Pipeline = nullptr;

	MTArray<VkDescriptorSet> m_DescriptorSets;
};

class MTVulkanTexture : public MTRDITexture
{
public:
	virtual ~MTVulkanTexture()
	{
		if (m_ImageView)
		{
			vkDestroyImageView(m_Device, m_ImageView, nullptr);
		}

		if (m_Image)
		{
			vkDestroyImage(m_Device, m_Image, nullptr);
		}

		if (m_ImageMemory)
		{
			vkFreeMemory(m_Device, m_ImageMemory, nullptr);
		}
	}

	void Init(VkDevice Device, VkPhysicalDevice PhysicalDevice, MTUInt32 Width, MTUInt32 Height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImageAspectFlags AspectFlags)
	{
		m_Device = Device;

		CreateImage(PhysicalDevice, Width, Height, Format, Tiling, Usage, Properties);
		CreateImageView(m_Image, Format, AspectFlags);
	}

	VkImage GetImage() const { return m_Image; }
	VkImageView GetImageView() const { return m_ImageView; }

private:
	void CreateImage(VkPhysicalDevice PhysicalDevice, MTUInt32 Width, MTUInt32 Height, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties)
	{
		VkImageCreateInfo ImageInfo = {};
		ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageInfo.extent.width = Width;
		ImageInfo.extent.height = Height;
		ImageInfo.extent.depth = 1;
		ImageInfo.mipLevels = 1;
		ImageInfo.arrayLayers = 1;
		ImageInfo.format = Format;
		ImageInfo.tiling = Tiling;
		ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageInfo.usage = Usage;
		ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(m_Device, &ImageInfo, nullptr, &m_Image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements MemRequirements;
		vkGetImageMemoryRequirements(m_Device, m_Image, &MemRequirements);

		VkMemoryAllocateInfo AllocInfo = {};
		AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocInfo.allocationSize = MemRequirements.size;
		AllocInfo.memoryTypeIndex = MTVulkanUtils::FindMemoryType(PhysicalDevice, MemRequirements.memoryTypeBits, Properties);

		if (vkAllocateMemory(m_Device, &AllocInfo, nullptr, &m_ImageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(m_Device, m_Image, m_ImageMemory, 0);
	}

	void CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags)
	{
		VkImageViewCreateInfo ViewInfo = {};
		ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewInfo.image = Image;
		ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ViewInfo.format = Format;
		ViewInfo.subresourceRange.aspectMask = AspectFlags;
		ViewInfo.subresourceRange.baseMipLevel = 0;
		ViewInfo.subresourceRange.levelCount = 1;
		ViewInfo.subresourceRange.baseArrayLayer = 0;
		ViewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device, &ViewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
	}

	VkDevice m_Device = nullptr;
	VkImage m_Image = nullptr;
	VkDeviceMemory m_ImageMemory = nullptr;
	VkImageView m_ImageView = nullptr;
};
