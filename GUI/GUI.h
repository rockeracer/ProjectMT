#pragma once

#ifdef GUI_EXPORTS
#define GUI_API MT_DLL_EXPORT
#else
#define GUI_API MT_DLL_IMPORT
#endif

#include "IGUIModule.h"
#include "Window.h"
