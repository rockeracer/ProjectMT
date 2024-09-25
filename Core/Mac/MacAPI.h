#pragma once

#include "IPlatformAPI.h"

class MTMacAPI : public IPlatformAPI
{
public:
    virtual MTModuleHandle LoadModule(const char* FilePath) override;
    virtual void DestroyModule(MTModuleHandle Module) override;
    virtual MTModuleFuncHandle GetModuleFunctionAddress(MTModuleHandle Module, const char* FuncName) override;
    
    virtual const char* GetModuleFilePrefix() override;
    virtual const char* GetModuleFileExt() override;
};
