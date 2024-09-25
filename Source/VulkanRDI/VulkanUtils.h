#pragma once

class MTVulkanUtils
{
public:
	static MTUInt32 FindMemoryType(VkPhysicalDevice PhysicalDevice, MTUInt32 TypeFilter, VkMemoryPropertyFlags Properties)
	{
		VkPhysicalDeviceMemoryProperties MemProperties;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);

		for (MTUInt32 i = 0; i < MemProperties.memoryTypeCount; i++)
		{
			if ((TypeFilter & (1 << i)) &&
				((MemProperties.memoryTypes[i].propertyFlags & Properties) == Properties))
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	static bool IsStencilFormat(VkFormat Format)
	{
		return (Format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (Format == VK_FORMAT_D24_UNORM_S8_UINT);
	}
};