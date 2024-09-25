#include "CoreObject.h"
#include "CoreObjectModule.h"

extern "C" COREOBJECT_API IModule* CreateModule()
{
    return new MTCoreObjectModule;
}
