#pragma once

#include "IRDI.h"
#include "VulkanShaderCodeFormat.h"

#include "shaderc/shaderc.h"

class MTVulkanMaterial;
class MTVulkanBuffer;
class MTVulkanTexture;
class MTVulkanAssetCommandBuffer;

struct MTVulkanQueueFamilyIndices
{
	MTInt32 GraphicsQueueIndex = -1;
	MTInt32 PresentQueueIndex = -1;

	bool IsValid() const
	{
		return (GraphicsQueueIndex >= 0) && (PresentQueueIndex >= 0);
	}
};

struct MTVulkanSwapChainSupport
{
	VkSurfaceCapabilitiesKHR Capabilities;
	MTArray<VkSurfaceFormatKHR> Formats;
	MTArray<VkPresentModeKHR> PresentModes;
};

struct MTVulkanSwapChain
{
	VkSwapchainKHR SwapChain = nullptr;

	MTInt32 ImageCount;
	VkFormat ImageFormat;
	VkExtent2D Extent;

	MTArray<VkImage> Images;
	VkRenderPass RenderPass;

	struct FrameData
	{
		VkImageView ImageView;
		VkFramebuffer Framebuffer;
	};

	MTArray<MTVulkanSwapChain::FrameData> FrameDatas;

	MTVulkanTexture* DepthBuffer = nullptr;
};

class MTVulkanRDI : public IRDI
{
public:
	MTVulkanRDI();
	virtual ~MTVulkanRDI();

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
	virtual MTRDIBuffer* CreateIndexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 IndexSize) override;
	virtual MTRDIDepthStencilState* CreateDepthStencilState(MTCompareFunction CompareFunction, bool bDepthWrite) override;
	virtual MTRDITexture* CreateTextureFromFilePath(const MTString& FilePath) override;
	virtual MTRDITexture* CreateTextureFromMemory(MTUInt32 Width, MTUInt32 Height, MTUChar* Pixels, MTUInt32 Size) override;
	virtual MTRDIMaterial* CreateMaterial(const MTString& VertexShaderCode, const MTString& PixelShaderCode, const MTArray<MTRDITexture*> Textures) override;

	virtual IShaderCodeFormat* GetShaderCodeFormat() override { return &m_ShaderCodeFormat; }


	VkSurfaceKHR CreateSurface(void* Handle);
	void DestroySurface(VkSurfaceKHR Surface);

	MTVulkanSwapChain CreateSwapChain(VkSurfaceKHR Surface, MTInt32 BackbufferCount, MTUInt32 Width, MTUInt32 Height);
	void DestroySwapChain(MTVulkanSwapChain& SwapChain);

	VkDescriptorSetLayout CreateDescriptorSetLayout(MTInt32 TextureCount);
	void DestroyDescriptorSetLayout(VkDescriptorSetLayout DescriptorSetLayout);

	VkPipelineLayout CreatePipelineLayout(MTInt32 SetLayoutCount, VkDescriptorSetLayout* SetLayouts);
	void DestroyPipelineLayout(VkPipelineLayout PipelineLayout);

	VkRenderPass CreateRenderPass(VkFormat ColorFormat);
	void DestroyRenderPass(VkRenderPass RenderPass);

	VkPipeline CreateGraphicsPipeline(const VkPipelineShaderStageCreateInfo* ShaderStages, VkPipelineLayout PipelineLayout);
	void DestroyGraphicsPipeline(VkPipeline Pipeline);

	VkCommandPool CreateCommandPool(VkSurfaceKHR Surface);

	void AllocateDescriptorSets(VkDescriptorSetLayout DescriptorSetLayout, const MTArray<MTRDITexture*> Textures, MTArray<VkDescriptorSet>& outDescriptorSets);

	VkShaderModule CreateShaderModule(const MTString& ShaderCode, shaderc_shader_kind Kind);
	void DestroyShaderModule(VkShaderModule ShaderModule);

	MTVulkanBuffer* CreateConstantBuffer(MTInt32 Size);

	MTVulkanTexture* CreateDepthBuffer(MTUInt32 Width, MTUInt32 Height);

	VkDevice GetDevice() const { return m_Device; }

private:
	void CreateInstance();

	bool CheckValidationLayerSupport();

	void PickPhysicalDevice(VkSurfaceKHR Surface);

	bool IsDeviceSuitable(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);

	bool CheckDeviceExtensionSupport(VkPhysicalDevice PhysicalDevice);

	void CreateLogicalDevice(VkSurfaceKHR Surface);

	void CreateDescriptorPool();

	VkSampler CreateTextureSampler();

	MTVulkanQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);

	MTVulkanSwapChainSupport QuerySwapChainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const MTArray<VkSurfaceFormatKHR>& AvailableFormats);

	VkPresentModeKHR ChooseSwapPresentMode(const MTArray<VkPresentModeKHR>& AvailablePresentModes);

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, MTUInt32 Width, MTUInt32 Height);

	VkFormat SelectSupportedImageFormat(const MTArray<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features);

	VkFormat SelectDepthBufferFormat();

	VkInstance m_VulkanInstance = nullptr;
	VkPhysicalDevice m_PhysicalDevice = nullptr;
	VkDevice m_Device = nullptr;
	VkQueue m_GraphicsQueue = nullptr;
	VkQueue m_PresentQueue = nullptr;

	VkCommandPool m_CommandPool = nullptr;
	VkDescriptorPool m_DescriptorPool = nullptr;

	MTVulkanAssetCommandBuffer* m_AssetCommandBuffer = nullptr;

	MTVulkanBuffer* m_ConstantBuffer = nullptr;

	VkSampler m_DefaultTextureSampler = nullptr;

	MTVulkanShaderCodeFormat m_ShaderCodeFormat;
};
