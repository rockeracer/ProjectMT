#pragma once

typedef void* MTModuleHandle;
typedef void* MTModuleFuncHandle;

class IPlatformAPI
{
public:
    virtual ~IPlatformAPI()	{}
    
    virtual MTModuleHandle LoadModule(const char* FilePath) = 0;
    virtual void DestroyModule(MTModuleHandle Module) = 0;
    virtual MTModuleFuncHandle GetModuleFunctionAddress(MTModuleHandle Module, const char* FuncName) = 0;
    
    virtual const char* GetModuleFilePrefix() = 0;
    virtual const char* GetModuleFileExt() = 0;
};
