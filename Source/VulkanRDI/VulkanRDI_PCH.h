#ifndef VULKANRDI_pch
#define VULKANRDI_pch

#include "Core.h"
#include "IRDI.h"

#include "VulkanRDI.h"

#include "vulkan/vulkan.h"

#ifdef _WIN32
	#include "windows.h"
	#include "vulkan/vulkan_win32.h"
#elif __APPLE__
	#include "TargetConditionals.h"
	#if TARGET_OS_IPHONE
		#include "vulkan/vulkan_ios.h"
	#elif TARGET_OS_MAC
		#include "vulkan/vulkan_macos.h"
	#endif
#endif

#include "VulkanUtils.h"

#endif /* VULKANRDI_pch */
