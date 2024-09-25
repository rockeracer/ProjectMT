#pragma once

#include "IModuleManager.h"

class MTModuleManager : public IModuleManager
{
public:
    MTModuleManager();
    virtual ~MTModuleManager();
    
    virtual void Init() override;
    virtual void Destroy() override;

    virtual IModule* LoadModule(const MTName& ModuleName) override;

private:
    MTArray<IModule*> m_Modules;
    MTMap<MTName, IModule*> m_ModulesMap;
};
