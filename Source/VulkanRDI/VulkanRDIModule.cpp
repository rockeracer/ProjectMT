#include "VulkanRDIModule.h"
#include "VulkanResource.h"
#include "VulkanCommandBuffer.h"
#include "VulkanAssetCommandBuffer.h"

#include "File.h"

const MTArray<const char*> DeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const MTArray<const char*> ValidationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool EnableValidationLayers = false;
#else
const bool EnableValidationLayers = true;
#endif

shaderc_include_result* Shaderc_Include_Resolve(void* user_data, const char* requested_source, int type,
	const char* requesting_source, size_t include_depth)
{
	MTString FileName = MTString("../../Shader/") + requested_source;
	char* FileNameStr = new char[FileName.Length() + 1];
	strcpy_s(FileNameStr, FileName.Length() + 1, FileName.c_str());

	MTFile IncludeFile(FileName, MTFile::Read);
	MTString FileContent = IncludeFile.ToString();
	IncludeFile.Close();
	char* FileContentStr = new char[FileContent.Length() + 1];
	strcpy_s(FileContentStr, FileContent.Length() + 1, FileContent.c_str());


	shaderc_include_result* Result = new shaderc_include_result;
	Result->source_name = FileNameStr;
	Result->source_name_length = FileName.Length();
	Result->content = FileContentStr;
	Result->content_length = FileContent.Length();
	Result->user_data = user_data;
	return Result;
}

void Shaderc_Include_Result_Release(void* user_data, shaderc_include_result* include_result)
{
	delete[] include_result->source_name;
	delete[] include_result->content;
	delete include_result;
}

MTVulkanRDI::MTVulkanRDI()
{
}

MTVulkanRDI::~MTVulkanRDI()
{
}

void MTVulkanRDI::Init()
{
	CreateInstance();

	VkSurfaceKHR DummySurface = CreateSurface(nullptr);

	PickPhysicalDevice(DummySurface);
	CreateLogicalDevice(DummySurface);
	m_CommandPool = CreateCommandPool(DummySurface);

	DestroySurface(DummySurface);

	CreateDescriptorPool();

	m_ConstantBuffer = CreateConstantBuffer(128);

	m_AssetCommandBuffer = new MTVulkanAssetCommandBuffer(m_Device, m_CommandPool, m_GraphicsQueue);

	m_DefaultTextureSampler = CreateTextureSampler();
}

void MTVulkanRDI::CreateInstance()
{
	if (EnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "MutantEngine";
	AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.pEngineName = "Mutant";
	AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &AppInfo;

#ifdef _WIN32
	const char* Extension[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
	CreateInfo.enabledExtensionCount = 2;
	CreateInfo.ppEnabledExtensionNames = Extension;
#endif

	if (EnableValidationLayers)
	{
		CreateInfo.enabledLayerCount = ValidationLayers.Num();
		CreateInfo.ppEnabledLayerNames = ValidationLayers.Data();
	}
	else
	{
		CreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&CreateInfo, nullptr, &m_VulkanInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create Vulkan instance!");
	}

// 	uint32_t ExtensionCount = 0;
// 	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);
// 	std::vector<VkExtensionProperties> Extensions(ExtensionCount);
// 	vkEnumerateInstanceExtensionProperties(nullptr, &Extensions, Extensions.data());
}

bool MTVulkanRDI::CheckValidationLayerSupport()
{
	MTUInt32 LayerCount;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	for (const char* LayerName : ValidationLayers)
	{
		bool LayerFound = false;

		for (const VkLayerProperties& LayerProperties : AvailableLayers)
		{
			if (strcmp(LayerName, LayerProperties.layerName) == 0)
			{
				LayerFound = true;
				break;
			}
		}

		if (!LayerFound)
		{
			return false;
		}
	}

	return true;
}

void MTVulkanRDI::PickPhysicalDevice(VkSurfaceKHR Surface)
{
	MTUInt32 DeviceCount = 0;
	vkEnumeratePhysicalDevices(m_VulkanInstance, &DeviceCount, nullptr);
	if (DeviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	MTArray<VkPhysicalDevice> Devices(DeviceCount);
	vkEnumeratePhysicalDevices(m_VulkanInstance, &DeviceCount, Devices.Data());

	for (const auto& Device : Devices)
	{
		if (IsDeviceSuitable(Device, Surface))
		{
			m_PhysicalDevice = Device;
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable Vulkan GPU!");
	}
}

bool MTVulkanRDI::IsDeviceSuitable(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
	bool bQueueSupported = FindQueueFamilies(PhysicalDevice, Surface).IsValid();
	bool bExtensionSupported = CheckDeviceExtensionSupport(PhysicalDevice);
	
	bool bSwapChainSupported = false;
	if (bExtensionSupported)
	{
		MTVulkanSwapChainSupport SwapChainSupport = QuerySwapChainSupport(PhysicalDevice, Surface);
		bSwapChainSupported = !SwapChainSupport.Formats.IsEmpty() && !SwapChainSupport.PresentModes.IsEmpty();
	}

	VkPhysicalDeviceFeatures SupportedFeatures;
	vkGetPhysicalDeviceFeatures(PhysicalDevice, &SupportedFeatures);

	return bQueueSupported && bExtensionSupported && bSwapChainSupported && SupportedFeatures.samplerAnisotropy;

// 	VkPhysicalDeviceProperties DeviceProperties;
// 	VkPhysicalDeviceFeatures DeviceFeatures;
// 	vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);
// 	vkGetPhysicalDeviceFeatures(PhysicalDevice, &DeviceFeatures);
}

bool MTVulkanRDI::CheckDeviceExtensionSupport(VkPhysicalDevice PhysicalDevice)
{
	MTUInt32 ExtensionCount;
	vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, nullptr);

	MTArray<VkExtensionProperties> AvailableExtensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.Data());

	MTArray<MTString> AvailableExtensionaNames(ExtensionCount);
	for (const VkExtensionProperties& Extension : AvailableExtensions)
	{
		AvailableExtensionaNames.Add(Extension.extensionName);
	};
	
	for (const char* Extension : DeviceExtensions)
	{
		if (AvailableExtensionaNames.Contain(Extension) == false)
		{
			return false;
		}
	}

	return true;
}

MTVulkanQueueFamilyIndices MTVulkanRDI::FindQueueFamilies(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
	MTUInt32 QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

	MTArray<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.Data());

	MTVulkanQueueFamilyIndices QueueFamilyIndices;

	MTInt32 i = 0;
	for (const auto& QueueFamily : QueueFamilies)
	{
		if ((QueueFamily.queueCount > 0) && (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			QueueFamilyIndices.GraphicsQueueIndex = i;
		}

		VkBool32 bSupportPresent = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &bSupportPresent);

		if ((QueueFamily.queueCount > 0) && bSupportPresent)
		{
			QueueFamilyIndices.PresentQueueIndex = i;
		}

		if (QueueFamilyIndices.IsValid())
		{
			break;
		}

		++i;
	}

	return QueueFamilyIndices;
}

MTVulkanSwapChainSupport MTVulkanRDI::QuerySwapChainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
	MTVulkanSwapChainSupport SwapChainSupport;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SwapChainSupport.Capabilities);

	MTUInt32 FormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, nullptr);

	if (FormatCount != 0)
	{
		SwapChainSupport.Formats.Resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, SwapChainSupport.Formats.Data());
	}

	MTUInt32 PresentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr);

	if (PresentModeCount != 0)
	{
		SwapChainSupport.PresentModes.Resize(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, SwapChainSupport.PresentModes.Data());
	}

	return SwapChainSupport;
}

VkSurfaceFormatKHR MTVulkanRDI::ChooseSwapSurfaceFormat(const MTArray<VkSurfaceFormatKHR>& AvailableFormats)
{
	for (const auto& AvailableFormat : AvailableFormats)
	{
		if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return AvailableFormat;
		}
	}

	return AvailableFormats[0];
}

VkPresentModeKHR MTVulkanRDI::ChooseSwapPresentMode(const MTArray<VkPresentModeKHR>& AvailablePresentModes)
{
	for (const auto& AvailablePresentMode : AvailablePresentModes)
	{
		if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return AvailablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D MTVulkanRDI::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, MTUInt32 Width, MTUInt32 Height)
{
	if (Capabilities.currentExtent.width != MTMath::NumMax<MTUInt32>())
	{
		return Capabilities.currentExtent;
	}
	else
	{
		VkExtent2D ActualExtent = { (MTUInt32)Width, Height };

		ActualExtent.width = MTMath::Clamp(ActualExtent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
		ActualExtent.height = MTMath::Clamp(ActualExtent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

		return ActualExtent;
	}
}

VkFormat MTVulkanRDI::SelectSupportedImageFormat(const MTArray<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
{
	for (VkFormat Format : Candidates)
	{
		VkFormatProperties Props;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, Format, &Props);

		if (Tiling == VK_IMAGE_TILING_LINEAR && (Props.linearTilingFeatures & Features) == Features)
		{
			return Format;
		}
		else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (Props.optimalTilingFeatures & Features) == Features)
		{
			return Format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat MTVulkanRDI::SelectDepthBufferFormat()
{
	return SelectSupportedImageFormat( { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
}

void MTVulkanRDI::CreateLogicalDevice(VkSurfaceKHR Surface)
{
	MTVulkanQueueFamilyIndices QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, Surface);

	MTArray<VkDeviceQueueCreateInfo> QueueCreateInfos;
	float QueuePriority = 1.0f;

	VkDeviceQueueCreateInfo GraphicsQueueCreateInfo = {};
	GraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	GraphicsQueueCreateInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsQueueIndex;
	GraphicsQueueCreateInfo.queueCount = 1;
	GraphicsQueueCreateInfo.pQueuePriorities = &QueuePriority;
	QueueCreateInfos.Add(GraphicsQueueCreateInfo);

	if (QueueFamilyIndices.PresentQueueIndex != QueueFamilyIndices.GraphicsQueueIndex)
	{
		VkDeviceQueueCreateInfo PresentQueueCreateInfo = GraphicsQueueCreateInfo;
		PresentQueueCreateInfo.queueFamilyIndex = QueueFamilyIndices.PresentQueueIndex;
		QueueCreateInfos.Add(PresentQueueCreateInfo);
	}

	VkPhysicalDeviceFeatures DeviceFeatures = {};
	DeviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo DeviceCreateInfo = {};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.queueCreateInfoCount = QueueCreateInfos.Num();
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.Data();
	DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
	DeviceCreateInfo.enabledExtensionCount = DeviceExtensions.Num();
	DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.Data();
	DeviceCreateInfo.enabledLayerCount = 0;

	if (vkCreateDevice(m_PhysicalDevice, &DeviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan logical device!");
	}

	vkGetDeviceQueue(m_Device, QueueFamilyIndices.GraphicsQueueIndex, 0, &m_GraphicsQueue);
	if (QueueFamilyIndices.PresentQueueIndex != QueueFamilyIndices.GraphicsQueueIndex)
	{
		vkGetDeviceQueue(m_Device, QueueFamilyIndices.PresentQueueIndex, 0, &m_PresentQueue);
	}
	else
	{
		m_PresentQueue = m_GraphicsQueue;
	}
}

void MTVulkanRDI::CreateDescriptorPool()
{
	VkDescriptorPoolSize PoolSize[2] = {};
	PoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	PoolSize[0].descriptorCount =  static_cast<MTUInt32>(MTRenderVar::RENDER_FRAME_COUNT) * 2;
	PoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	PoolSize[1].descriptorCount = static_cast<MTUInt32>(MTRenderVar::RENDER_FRAME_COUNT) * 2 * 2;

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.poolSizeCount = 2;
	PoolInfo.pPoolSizes = PoolSize;
	PoolInfo.maxSets = static_cast<MTUInt32>(MTRenderVar::RENDER_FRAME_COUNT) * 2;

	if (vkCreateDescriptorPool(m_Device, &PoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

VkSampler MTVulkanRDI::CreateTextureSampler()
{
	VkSamplerCreateInfo SamplerInfo = {};
	SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInfo.magFilter = VK_FILTER_LINEAR;
	SamplerInfo.minFilter = VK_FILTER_LINEAR;
	SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInfo.anisotropyEnable = VK_TRUE;
	SamplerInfo.maxAnisotropy = 16;
	SamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerInfo.unnormalizedCoordinates = VK_FALSE;
	SamplerInfo.compareEnable = VK_FALSE;
	SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	VkSampler Sampler = nullptr;
	if (vkCreateSampler(m_Device, &SamplerInfo, nullptr, &Sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}

	return Sampler;
}

void MTVulkanRDI::AllocateDescriptorSets(VkDescriptorSetLayout DescriptorSetLayout, const MTArray<MTRDITexture*> Textures, MTArray<VkDescriptorSet>& outDescriptorSets)
{
	const MTArray<VkBuffer>& UniformBuffer = m_ConstantBuffer->GetBuffers();
	MTInt32 BufferSize = m_ConstantBuffer->GetBufferSize();

	MTInt32 DescriptorSetCount = UniformBuffer.Num();

	MTArray<VkDescriptorSetLayout> Layouts(DescriptorSetCount, DescriptorSetLayout);
	VkDescriptorSetAllocateInfo AllocInfo = {};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.descriptorPool = m_DescriptorPool;
	AllocInfo.descriptorSetCount = DescriptorSetCount;
	AllocInfo.pSetLayouts = Layouts.Data();

	outDescriptorSets.Resize(DescriptorSetCount);
	VkResult AllocateResult = vkAllocateDescriptorSets(m_Device, &AllocInfo, outDescriptorSets.Data());
	if (AllocateResult != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (MTInt32 i = 0; i < DescriptorSetCount; i++)
	{
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = UniformBuffer[i];
		BufferInfo.offset = 0;
		BufferInfo.range = BufferSize;

		VkWriteDescriptorSet UniformBufferDescriptor = {};
		UniformBufferDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		UniformBufferDescriptor.dstSet = outDescriptorSets[i];
		UniformBufferDescriptor.dstBinding = 0;
		UniformBufferDescriptor.dstArrayElement = 0;
		UniformBufferDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UniformBufferDescriptor.descriptorCount = 1;
		UniformBufferDescriptor.pBufferInfo = &BufferInfo;

		if (Textures.IsEmpty())
		{
			vkUpdateDescriptorSets(m_Device, 1, &UniformBufferDescriptor, 0, nullptr);
		}
		else
		{
			MTArray<VkWriteDescriptorSet> DescriptorWrites(1 + Textures.Num(), {});
			DescriptorWrites[0] = UniformBufferDescriptor;

			MTArray<VkDescriptorImageInfo> ImageInfos(Textures.Num(), {});

			MTInt32 iTexture = 0;
			for (MTRDITexture* Texture : Textures)
			{
				MTVulkanTexture* VulkanTexture = dynamic_cast<MTVulkanTexture*>(Texture);
				if (VulkanTexture == nullptr)
				{
					throw std::runtime_error("this is not vulkan texture!");
				}

				ImageInfos[iTexture].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				ImageInfos[iTexture].imageView = VulkanTexture->GetImageView();
				ImageInfos[iTexture].sampler = m_DefaultTextureSampler;

				MTInt32 iDescriptor = iTexture + 1;
				DescriptorWrites[iDescriptor].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				DescriptorWrites[iDescriptor].dstSet = outDescriptorSets[i];
				DescriptorWrites[iDescriptor].dstBinding = iDescriptor;
				DescriptorWrites[iDescriptor].dstArrayElement = 0;
				DescriptorWrites[iDescriptor].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				DescriptorWrites[iDescriptor].descriptorCount = 1;
				DescriptorWrites[iDescriptor].pImageInfo = &ImageInfos[iTexture];

				++iTexture;
			}

			vkUpdateDescriptorSets(m_Device, DescriptorWrites.Num(), DescriptorWrites.Data(), 0, nullptr);
		}
	}
}

void MTVulkanRDI::Destroy()
{
	if (m_ConstantBuffer)
	{
		delete m_ConstantBuffer;
	}

	if (m_AssetCommandBuffer)
	{
		delete m_AssetCommandBuffer;
	}

	if (m_DefaultTextureSampler)
	{
		vkDestroySampler(m_Device, m_DefaultTextureSampler, nullptr);
	}

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	
	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_VulkanInstance, nullptr);
}

MTRDIWindowView* MTVulkanRDI::CreateWindowView(void* Handle, MTUInt32 Width, MTUInt32 Height, MTInt32 BackbufferCount)
{
	MTVulkanWindowView* WindowView = new MTVulkanWindowView(Width, Height);
	WindowView->Init(this, Handle, BackbufferCount, Width, Height);
	return WindowView;
}

VkSurfaceKHR MTVulkanRDI::CreateSurface(void* Handle)
{
	VkSurfaceKHR Surface = nullptr;

#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};
	SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	SurfaceCreateInfo.hwnd = (HWND)Handle;
	SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(m_VulkanInstance, &SurfaceCreateInfo, nullptr, &Surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan surface!");
	}
#else
	throw std::runtime_error("This platform is not supported yet!");
#endif

	return Surface;
}

void MTVulkanRDI::DestroySurface(VkSurfaceKHR Surface)
{
	if (Surface)
	{
		vkDestroySurfaceKHR(m_VulkanInstance, Surface, nullptr);
	}
}

MTVulkanSwapChain MTVulkanRDI::CreateSwapChain(VkSurfaceKHR Surface, MTInt32 BackbufferCount, MTUInt32 Width, MTUInt32 Height)
{
	MTVulkanSwapChainSupport SwapChainSupport = QuerySwapChainSupport(m_PhysicalDevice, Surface);

	VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
	VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainSupport.PresentModes);
	VkExtent2D Extent = ChooseSwapExtent(SwapChainSupport.Capabilities, Width, Height);
	
	MTUInt32 ImageCount = SwapChainSupport.Capabilities.minImageCount + 1;
	if ((SwapChainSupport.Capabilities.maxImageCount > 0) && (ImageCount > SwapChainSupport.Capabilities.maxImageCount))
	{
		ImageCount = SwapChainSupport.Capabilities.maxImageCount;
	}
	ImageCount = MTMath::Min(ImageCount, (MTUInt32)BackbufferCount);

	VkSwapchainCreateInfoKHR CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	CreateInfo.surface = Surface;
	CreateInfo.minImageCount = ImageCount;
	CreateInfo.imageFormat = SurfaceFormat.format;
	CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	CreateInfo.imageExtent = Extent;
	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	MTVulkanQueueFamilyIndices QueueIndices = FindQueueFamilies(m_PhysicalDevice, Surface);
	MTUInt32 QueueFamilyIndices[] = { (MTUInt32)QueueIndices.GraphicsQueueIndex, (MTUInt32)QueueIndices.PresentQueueIndex };

	if (QueueIndices.GraphicsQueueIndex != QueueIndices.PresentQueueIndex)
	{
		CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		CreateInfo.queueFamilyIndexCount = 2;
		CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
	}
	else
	{
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount = 0; // Optional
		CreateInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	CreateInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
	CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	CreateInfo.presentMode = PresentMode;
	CreateInfo.clipped = VK_TRUE;
	CreateInfo.oldSwapchain = VK_NULL_HANDLE;

	MTVulkanSwapChain SwapChain;
	SwapChain.ImageCount = ImageCount;
	SwapChain.ImageFormat = SurfaceFormat.format;
	SwapChain.Extent = Extent;

	if (vkCreateSwapchainKHR(m_Device, &CreateInfo, nullptr, &SwapChain.SwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vulkan swap chain!");
	}

	vkGetSwapchainImagesKHR(m_Device, SwapChain.SwapChain, &ImageCount, nullptr);
	SwapChain.Images.Resize(ImageCount);
	vkGetSwapchainImagesKHR(m_Device, SwapChain.SwapChain, &ImageCount, SwapChain.Images.Data());

	SwapChain.RenderPass = CreateRenderPass(SurfaceFormat.format);

	SwapChain.DepthBuffer = CreateDepthBuffer(Extent.width, Extent.height);

	SwapChain.FrameDatas.Resize(ImageCount);
	for (MTUInt32 i = 0; i < ImageCount; ++i)
	{
		VkImageViewCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		CreateInfo.image = SwapChain.Images[i];
		CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		CreateInfo.format = SwapChain.ImageFormat;
		CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		CreateInfo.subresourceRange.baseMipLevel = 0;
		CreateInfo.subresourceRange.levelCount = 1;
		CreateInfo.subresourceRange.baseArrayLayer = 0;
		CreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device, &CreateInfo, nullptr, &SwapChain.FrameDatas[i].ImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}


		VkImageView Attachments[] = { SwapChain.FrameDatas[i].ImageView, SwapChain.DepthBuffer->GetImageView() };

		VkFramebufferCreateInfo FramebufferInfo = {};
		FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferInfo.renderPass = SwapChain.RenderPass;
		FramebufferInfo.attachmentCount = 2;
		FramebufferInfo.pAttachments = Attachments;
		FramebufferInfo.width = Extent.width;
		FramebufferInfo.height = Extent.height;
		FramebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &FramebufferInfo, nullptr, &SwapChain.FrameDatas[i].Framebuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	return SwapChain;
}

void MTVulkanRDI::DestroySwapChain(MTVulkanSwapChain& SwapChain)
{
	for (MTVulkanSwapChain::FrameData FrameData : SwapChain.FrameDatas)
	{
		vkDestroyFramebuffer(m_Device, FrameData.Framebuffer, nullptr);
		vkDestroyImageView(m_Device, FrameData.ImageView, nullptr);
	}

	DestroyRenderPass(SwapChain.RenderPass);

	if (SwapChain.DepthBuffer)
	{
		delete SwapChain.DepthBuffer;
	}

	vkDestroySwapchainKHR(m_Device, SwapChain.SwapChain, nullptr);
}

VkDescriptorSetLayout MTVulkanRDI::CreateDescriptorSetLayout(MTInt32 TextureCount)
{
	MTArray<VkDescriptorSetLayoutBinding> Bindings(1 + TextureCount, {});
	
	Bindings[0].binding = 0;
	Bindings[0].descriptorCount = 1;
	Bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	Bindings[0].pImmutableSamplers = nullptr;
	Bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	for (MTInt32 i = 0; i < TextureCount; ++i)
	{
		Bindings[i + 1].binding = i + 1;
		Bindings[i + 1].descriptorCount = 1;
		Bindings[i + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		Bindings[i + 1].pImmutableSamplers = nullptr;
		Bindings[i + 1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	VkDescriptorSetLayoutCreateInfo LayoutInfo = {};
	LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	LayoutInfo.bindingCount = Bindings.Num();
	LayoutInfo.pBindings = Bindings.Data();

	VkDescriptorSetLayout DescriptorSetLayout;
	if (vkCreateDescriptorSetLayout(m_Device, &LayoutInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	return DescriptorSetLayout;
}

void MTVulkanRDI::DestroyDescriptorSetLayout(VkDescriptorSetLayout DescriptorSetLayout)
{
	if (DescriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(m_Device, DescriptorSetLayout, nullptr);
	}
}

VkPipelineLayout MTVulkanRDI::CreatePipelineLayout(MTInt32 SetLayoutCount, VkDescriptorSetLayout* SetLayouts)
{
	VkPipelineLayoutCreateInfo PipelineLayoutInfo = {};
	PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutInfo.setLayoutCount = SetLayoutCount;
	PipelineLayoutInfo.pSetLayouts = SetLayouts;
	PipelineLayoutInfo.pushConstantRangeCount = 0;
	PipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout PipelineLayout = nullptr;
	if (vkCreatePipelineLayout(m_Device, &PipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	return PipelineLayout;
}

void MTVulkanRDI::DestroyPipelineLayout(VkPipelineLayout PipelineLayout)
{
	if (PipelineLayout)
	{
		vkDestroyPipelineLayout(m_Device, PipelineLayout, nullptr);
	}
}

VkRenderPass MTVulkanRDI::CreateRenderPass(VkFormat ColorFormat)
{
	VkAttachmentDescription ColorAttachment = {};
	ColorAttachment.format = ColorFormat;
	ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription DepthAttachment = {};
	DepthAttachment.format = SelectDepthBufferFormat();
	DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ColorAttachmentRef = {};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthAttachmentRef = {};
	DepthAttachmentRef.attachment = 1;
	DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription Subpass = {};
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentRef;
	Subpass.pDepthStencilAttachment = &DepthAttachmentRef;

	VkSubpassDependency Dependency = {};
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0;
	Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.srcAccessMask = 0;
	Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription Attachments[] = { ColorAttachment, DepthAttachment };

	VkRenderPassCreateInfo RenderPassInfo = {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.attachmentCount = 2;
	RenderPassInfo.pAttachments = Attachments;
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &Subpass;
	RenderPassInfo.pDependencies = &Dependency;


	VkRenderPass RenderPass;
	if (vkCreateRenderPass(m_Device, &RenderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

	return RenderPass;
}

void MTVulkanRDI::DestroyRenderPass(VkRenderPass RenderPass)
{
	if (RenderPass)
	{
		vkDestroyRenderPass(m_Device, RenderPass, nullptr);
	}
}

VkPipeline MTVulkanRDI::CreateGraphicsPipeline(const VkPipelineShaderStageCreateInfo* ShaderStages, VkPipelineLayout PipelineLayout)
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = 44;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription AttributeDescriptions[4];
	AttributeDescriptions[0].binding = 0;
	AttributeDescriptions[0].location = 0;
	AttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	AttributeDescriptions[0].offset = 0;
	AttributeDescriptions[1].binding = 0;
	AttributeDescriptions[1].location = 1;
	AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	AttributeDescriptions[1].offset = 12;
	AttributeDescriptions[2].binding = 0;
	AttributeDescriptions[2].location = 2;
	AttributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	AttributeDescriptions[2].offset = 24;
	AttributeDescriptions[3].binding = 0;
	AttributeDescriptions[3].location = 3;
	AttributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	AttributeDescriptions[3].offset = 36;

	VkPipelineVertexInputStateCreateInfo VertexInputInfo = {};
	VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputInfo.vertexBindingDescriptionCount = 1;
	VertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	VertexInputInfo.vertexAttributeDescriptionCount = 4;
	VertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo InputAssembly = {};
	InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo Rasterizer = {};
	Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	Rasterizer.depthClampEnable = VK_FALSE;
	Rasterizer.rasterizerDiscardEnable = VK_FALSE;
	Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	Rasterizer.lineWidth = 1.0f;
	Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	Rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	Rasterizer.depthBiasEnable = VK_FALSE;
	Rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	Rasterizer.depthBiasClamp = 0.0f; // Optional
	Rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo Multisampling = {};
	Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	Multisampling.sampleShadingEnable = VK_FALSE;
	Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	Multisampling.minSampleShading = 1.0f; // Optional
	Multisampling.pSampleMask = nullptr; // Optional
	Multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	Multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineDepthStencilStateCreateInfo DepthStencil = {};
	DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencil.depthTestEnable = VK_TRUE;
	DepthStencil.depthWriteEnable = VK_TRUE;
	DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	DepthStencil.depthBoundsTestEnable = VK_FALSE;
	DepthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
	ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachment.blendEnable = VK_FALSE;
	ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo ColorBlending = {};
	ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlending.logicOpEnable = VK_FALSE;
	ColorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	ColorBlending.attachmentCount = 1;
	ColorBlending.pAttachments = &ColorBlendAttachment;
	ColorBlending.blendConstants[0] = 0.0f; // Optional
	ColorBlending.blendConstants[1] = 0.0f; // Optional
	ColorBlending.blendConstants[2] = 0.0f; // Optional
	ColorBlending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState DynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo DynamicState = {};
	DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	DynamicState.dynamicStateCount = 2;
	DynamicState.pDynamicStates = DynamicStates;

	VkViewport DummyViewport = {};
	DummyViewport.x = 0.0f;
	DummyViewport.y = 0.0f;
	DummyViewport.width = 1024;
	DummyViewport.height = 768;
	DummyViewport.minDepth = 0.0f;
	DummyViewport.maxDepth = 1.0f;

	VkRect2D DummyScissor = {};
	DummyScissor.offset = {0, 0};
	DummyScissor.extent = {1024, 768};
	VkPipelineViewportStateCreateInfo DummyViewportState = {};
	DummyViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	DummyViewportState.viewportCount = 1;
	DummyViewportState.pViewports = &DummyViewport;
	DummyViewportState.scissorCount = 1;
	DummyViewportState.pScissors = &DummyScissor;

	VkRenderPass DummyRenderPass = CreateRenderPass(VK_FORMAT_B8G8R8A8_UNORM);

	VkGraphicsPipelineCreateInfo PipelineInfo = {};
	PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineInfo.stageCount = 2;
	PipelineInfo.pStages = ShaderStages;
	PipelineInfo.pVertexInputState = &VertexInputInfo;
	PipelineInfo.pInputAssemblyState = &InputAssembly;
	PipelineInfo.pViewportState = &DummyViewportState;
	PipelineInfo.pRasterizationState = &Rasterizer;
	PipelineInfo.pMultisampleState = &Multisampling;
	PipelineInfo.pDepthStencilState = &DepthStencil;
	PipelineInfo.pColorBlendState = &ColorBlending;
	PipelineInfo.pDynamicState = &DynamicState;
	PipelineInfo.layout = PipelineLayout;
	PipelineInfo.renderPass = DummyRenderPass;
	PipelineInfo.subpass = 0;
	PipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	PipelineInfo.basePipelineIndex = -1; // Optional

	VkPipeline GraphicsPipeline;
	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	DestroyRenderPass(DummyRenderPass);

	return GraphicsPipeline;
}

void MTVulkanRDI::DestroyGraphicsPipeline(VkPipeline Pipeline)
{
	if (Pipeline)
	{
		vkDestroyPipeline(m_Device, Pipeline, nullptr);
	}
}

VkCommandPool MTVulkanRDI::CreateCommandPool(VkSurfaceKHR Surface)
{
	MTVulkanQueueFamilyIndices QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, Surface);

	VkCommandPoolCreateInfo CommandPoolCreateInfo = {};
	CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolCreateInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsQueueIndex;
	CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool CommandPool = nullptr;
	if (vkCreateCommandPool(m_Device, &CommandPoolCreateInfo, nullptr, &CommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}

	return CommandPool;
}

void* MTVulkanRDI::InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc, MTInt32 RenderFrameCount)
{
	return nullptr;
}

void MTVulkanRDI::ShutdownGUIRender()
{
}

void MTVulkanRDI::CommitRenderCommandBuffer()
{
}

void MTVulkanRDI::BeginRender()
{
	if (m_AssetCommandBuffer)
	{
		m_AssetCommandBuffer->CommitCommand();
	}
}

void MTVulkanRDI::RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData)
{
}

void MTVulkanRDI::EndRender(MTRDIWindowView* View)
{
	MTVulkanWindowView* VulkanView = dynamic_cast<MTVulkanWindowView*>(View);
	if (VulkanView)
	{
		VulkanView->Present(m_PresentQueue);
	}
}

void MTVulkanRDI::WaitForRenderCompleted()
{
	vkDeviceWaitIdle(m_Device);

	m_AssetCommandBuffer->ClearCommand();
}

MTRDIShader* MTVulkanRDI::CreateShaderWithSource(const MTString& Source, const MTString& VertexShaderFunction, const MTString& PixelShaderFunction)
{
	return nullptr;
}

IRDICommandBuffer* MTVulkanRDI::CreateCommandBuffer()
{
    return new MTVulkanCommandBuffer(m_Device, m_CommandPool, MTRenderVar::RENDER_FRAME_COUNT, m_GraphicsQueue, m_ConstantBuffer);
}

MTRDIBuffer* MTVulkanRDI::CreateVertexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 VertexSize)
{
	MTVulkanBuffer* StagingBuffer = new MTVulkanBuffer(BufferSize, VertexSize);
	StagingBuffer->Init(m_Device, m_PhysicalDevice, 1, BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	StagingBuffer->FillBuffer(0, Data);

	MTVulkanBuffer* VertexBuffer = new MTVulkanBuffer(BufferSize, VertexSize);
	VertexBuffer->Init(m_Device, m_PhysicalDevice, 1, BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	m_AssetCommandBuffer->UploadBuffer(StagingBuffer, VertexBuffer->GetBuffer(0));

	return VertexBuffer;
}

MTRDIBuffer* MTVulkanRDI::CreateIndexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 IndexSize)
{
	MTVulkanBuffer* StagingBuffer = new MTVulkanBuffer(BufferSize, IndexSize);
	StagingBuffer->Init(m_Device, m_PhysicalDevice, 1, BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	StagingBuffer->FillBuffer(0, Data);

	MTVulkanBuffer* IndexBuffer = new MTVulkanBuffer(BufferSize, IndexSize);
	IndexBuffer->Init(m_Device, m_PhysicalDevice, 1, BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	m_AssetCommandBuffer->UploadBuffer(StagingBuffer, IndexBuffer->GetBuffer(0));

	return IndexBuffer;
}

MTRDIDepthStencilState* MTVulkanRDI::CreateDepthStencilState(MTCompareFunction CompareFunction, bool bDepthWrite)
{
    return nullptr;
}

MTRDITexture* MTVulkanRDI::CreateTextureFromFilePath(const MTString& FilePath)
{
    return nullptr;
}

MTRDITexture* MTVulkanRDI::CreateTextureFromMemory(MTUInt32 Width, MTUInt32 Height, MTUChar* Pixels, MTUInt32 Size)
{
	MTVulkanBuffer* StagingBuffer = new MTVulkanBuffer(Size, Size);
	StagingBuffer->Init(m_Device, m_PhysicalDevice, 1, Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	StagingBuffer->FillBuffer(0, Pixels);

	MTVulkanTexture* Texture = new MTVulkanTexture;
	Texture->Init(m_Device, m_PhysicalDevice, Width, Height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

	m_AssetCommandBuffer->UploadTexture(StagingBuffer, Texture->GetImage(), VK_FORMAT_R8G8B8A8_UNORM, Width, Height);

	return Texture;
}

MTRDIMaterial* MTVulkanRDI::CreateMaterial(const MTString& VertexShaderCode, const MTString& PixelShaderCode, const MTArray<MTRDITexture*> Textures)
{
	MTVulkanMaterial* Material = new MTVulkanMaterial;
	Material->Init(this, VertexShaderCode, PixelShaderCode, Textures);
	return Material;
}

VkShaderModule MTVulkanRDI::CreateShaderModule(const MTString& ShaderCode, shaderc_shader_kind Kind)
{
	shaderc_compiler_t Compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t Option = shaderc_compile_options_initialize();
	shaderc_compile_options_set_include_callbacks(Option, &Shaderc_Include_Resolve, &Shaderc_Include_Result_Release, nullptr);

	shaderc_compilation_result_t CompileResult = shaderc_compile_into_spv(Compiler, ShaderCode.c_str(), ShaderCode.Length(), Kind, "ShaderFile.shd", "main", Option);

	const char* Error = shaderc_result_get_error_message(CompileResult);
	const char* ResultData = shaderc_result_get_bytes(CompileResult);
	const MTInt32 ResultDataSize = (MTInt32)shaderc_result_get_length(CompileResult);

	VkShaderModuleCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	CreateInfo.codeSize = ResultDataSize;
	CreateInfo.pCode = reinterpret_cast<const uint32_t*>(ResultData);

	VkShaderModule ShaderModule;
	if (vkCreateShaderModule(m_Device, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	shaderc_result_release(CompileResult);
	shaderc_compiler_release(Compiler);
	shaderc_compile_options_release(Option);

	return ShaderModule;
}

void MTVulkanRDI::DestroyShaderModule(VkShaderModule ShaderModule)
{
	vkDestroyShaderModule(m_Device, ShaderModule, nullptr);
}

MTVulkanBuffer* MTVulkanRDI::CreateConstantBuffer(MTInt32 Size)
{
	MTVulkanBuffer* ConstantBuffer = new MTVulkanBuffer(Size, Size);
	ConstantBuffer->Init(m_Device, m_PhysicalDevice, MTRenderVar::RENDER_FRAME_COUNT, Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

	return ConstantBuffer;
}

MTVulkanTexture* MTVulkanRDI::CreateDepthBuffer(MTUInt32 Width, MTUInt32 Height)
{
	VkFormat Format = SelectDepthBufferFormat();

	MTVulkanTexture* DepthBuffer = new MTVulkanTexture;
	DepthBuffer->Init(m_Device, m_PhysicalDevice, Width, Height, Format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

	m_AssetCommandBuffer->UploadDepthBuffer(DepthBuffer->GetImage(), Format);

	return DepthBuffer;
}
