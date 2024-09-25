#include "Engine.h"
#include "EngineModule.h"

extern "C" ENGINE_API IModule* CreateModule()
{
    return new MTEngineModule;
}
