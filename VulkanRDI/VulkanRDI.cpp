#include "VulkanRDI.h"
#include "VulkanRDIModule.h"

extern "C" VULKANRDI_API IRDI* CreateModule()
{
	return new MTVulkanRDI;
}