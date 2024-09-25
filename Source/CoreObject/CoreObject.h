#pragma once

#ifdef COREOBJECT_EXPORTS
#define COREOBJECT_API MT_DLL_EXPORT
#else
#define COREOBJECT_API MT_DLL_IMPORT
#endif

#include "Object.h"
#include "ClassRegister.h"

#define MTCLASS()
#define MTPROPERTY()
#define GENERATED_BODY(Class)\
	REGISTER_CLASS_##Class()