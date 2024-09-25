#pragma once

#define MT_DLL_EXPORT __declspec(dllexport)
#define MT_DLL_IMPORT __declspec(dllimport)

#define _USE_MATH_DEFINES

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251) // 'type' needs to have dll-interface to be used by clients of 'type'
#endif

