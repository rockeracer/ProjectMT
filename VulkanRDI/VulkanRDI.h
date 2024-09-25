#pragma once

#ifdef VULKANRDI_EXPORTS
#define VULKANRDI_API MT_DLL_EXPORT
#else
#define VULKANRDI_API MT_DLL_IMPORT
#endif
