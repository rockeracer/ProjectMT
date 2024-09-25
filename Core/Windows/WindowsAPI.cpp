#include "WindowsAPI.h"

#include "windows.h"

MTModuleHandle MTWindowsAPI::LoadModule(const char* FilePath)
{
    return LoadLibrary(FilePath);
}

void MTWindowsAPI::DestroyModule(MTModuleHandle Module)
{
	FreeLibrary((HMODULE)Module);
}

MTModuleFuncHandle MTWindowsAPI::GetModuleFunctionAddress(MTModuleHandle Module, const char* FuncName)
{
    return GetProcAddress((HMODULE)Module, FuncName);
}

const char* MTWindowsAPI::GetModuleFilePrefix()
{
    return "";
}

const char* MTWindowsAPI::GetModuleFileExt()
{
    return "dll";
}
