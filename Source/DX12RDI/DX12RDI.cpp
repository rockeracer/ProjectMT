#include "DX12RDI.h"
#include "DX12RDIModule.h"

extern "C" DX12RDI_API IRDI* CreateModule()
{
	return new MTDX12RDI;
}