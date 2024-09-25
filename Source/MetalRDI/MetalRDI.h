#pragma once

#ifdef METALRDI_EXPORTS
#define METALRDI_API MT_DLL_EXPORT
#else
#define METALRDI_API MT_DLL_IMPORT
#endif

#include "IRDI.h"
