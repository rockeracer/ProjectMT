#pragma once

#ifdef RDI_EXPORTS
#define RDI_API MT_DLL_EXPORT
#else
#define RDI_API MT_DLL_IMPORT
#endif

#include "RDIResource.h"
#include "IRDI.h"

namespace RDI
{
	extern "C" RDI_API IRDI* GetRDI();
}