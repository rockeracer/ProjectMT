#pragma once

#ifdef RENDERCORE_EXPORTS
#define RENDERCORE_API MT_DLL_EXPORT
#else
#define RENDERCORE_API MT_DLL_IMPORT
#endif

#include "IRenderCoreModule.h"
#include "RDIResource.h"
#include "RenderCommandBuffer.h"
