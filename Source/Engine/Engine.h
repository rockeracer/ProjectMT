#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API MT_DLL_EXPORT
#else
#define ENGINE_API MT_DLL_IMPORT
#endif
