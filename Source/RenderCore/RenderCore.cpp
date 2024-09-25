#include "RenderCore.h"
#include "RenderCoreModule.h"

extern "C" RENDERCORE_API IRenderCoreModule* CreateModule()
{
    return new MTRenderCoreModule;
}
