#pragma once

#include <iostream>
#include <cassert>
#include <thread>
#include <condition_variable>
#include <functional>

#ifdef _WIN32
#include "Windows/WindowsDef.h"
#elif __APPLE__
#include "Mac/Mac.h"
#endif

#ifdef CORE_EXPORTS
#define CORE_API MT_DLL_EXPORT
#else
#define CORE_API MT_DLL_IMPORT
#endif

#define MT_ASSERT assert
#define MT_ASSERT_EXPR(exp, msg) assert(exp && msg)



// typedef __int32_t MTInt32;
// typedef __int64_t MTInt64;
// typedef __uint32_t MTUInt32;
// typedef __uint64_t MTUInt64;
typedef int32_t MTInt32;
typedef int64_t MTInt64;
typedef uint32_t MTUInt32;
typedef uint64_t MTUInt64;
typedef unsigned char MTUChar;


#include "Array.h"
#include "Map.h"
#include "MutantString.h"
typedef MTString MTName;

typedef std::thread MTThread;
typedef std::mutex MTMutex;

template<class...Args>
using MTFunction = std::function<Args...>;


template<class T>
using MTLockGuard = std::lock_guard<T>;
template<class T>
using MTUniqueLock = std::unique_lock<T>;

typedef std::condition_variable MTConditionVariable;

#define MT_LOG printf


#include "IPlatformAPI.h"
#include "IModule.h"
#include "IModuleManager.h"

#include "Clock.h"
#include "Semaphore.h"
#include "MathLib.h"

typedef MTVector4 MTColor;

extern "C" CORE_API void InitCore();
extern "C" CORE_API void ShutdownCore();

extern "C" CORE_API IPlatformAPI* PlatformAPI();
extern "C" CORE_API IModuleManager* ModuleManager();

namespace Core
{
	template<class T = IModule>
	T* LoadModule(const MTName& ModuleName)
	{
		return dynamic_cast<T*>(ModuleManager()->LoadModule(ModuleName));
	}
}
