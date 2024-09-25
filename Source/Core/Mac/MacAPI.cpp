#include "Core.pch"
#include "MacAPI.h"

#include <dlfcn.h>

MTModuleHandle MTMacAPI::LoadModule(const char* FilePath)
{
    return dlopen(FilePath, 0);
}

void MTMacAPI::DestroyModule(MTModuleHandle Module)
{
    dlclose(Module);
}

MTModuleFuncHandle MTMacAPI::GetModuleFunctionAddress(MTModuleHandle Module, const char* FuncName)
{
    return dlsym(Module, FuncName);
}

const char* MTMacAPI::GetModuleFilePrefix()
{
    return "lib";
}

const char* MTMacAPI::GetModuleFileExt()
{
    return "dylib";
}
