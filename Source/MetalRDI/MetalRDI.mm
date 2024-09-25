#include "MetalRDI.pch"
#include "MetalRDI.h"
#include "MetalRDIModule.h"

extern "C" METALRDI_API IRenderAPI* CreateModule()
{
    return new MTMetalRDIModule;
}
