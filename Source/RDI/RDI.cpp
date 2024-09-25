#include "RDI.h"
#include "RDIModule.h"

extern "C" RDI_API IModule* CreateModule()
{
    return new MTRDIModule;
}

IRDI* RDI::GetRDI()
{
	MTRDIModule* RDIModule = Core::LoadModule<MTRDIModule>("RDI");
	if (RDIModule)
	{
		return RDIModule->GetRDI();
	}

	return nullptr;
}
