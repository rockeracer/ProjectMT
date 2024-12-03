#pragma once

class IModule;

class IModuleManager
{
public:
    IModuleManager() {}
    virtual ~IModuleManager() {}
    
    virtual void Init() = 0;
    virtual void Destroy() = 0;
    
    virtual IModule* LoadModule(const MTName& ModuleName) = 0;
};
