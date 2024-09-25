#include "ModuleManager.h"
#include "IModule.h"

const char* CREATE_MODULE_FUNC_NAME = "CreateModule";

MTModuleManager::MTModuleManager()
{
}


MTModuleManager::~MTModuleManager()
{
}

void MTModuleManager::Init()
{
    //LoadModule("Core");
}

void MTModuleManager::Destroy()
{
    for (MTInt32 i = m_Modules.Num() - 1; i >= 0; --i)
    {
        m_Modules[i]->Destroy();
        delete m_Modules[i];
    }
        
    m_Modules.Clear();
    m_ModulesMap.Clear();
}

IModule* MTModuleManager::LoadModule(const MTName& ModuleName)
{
    IModule** ExistModule = m_ModulesMap.Get(ModuleName);
    if (ExistModule)
    {
        return *ExistModule;
    }
    
    MTString ModulePath = PlatformAPI()->GetModuleFilePrefix() + ModuleName + "." + PlatformAPI()->GetModuleFileExt();
    
    MTModuleHandle hModule = PlatformAPI()->LoadModule(ModulePath.c_str());
    if (hModule)
    {
        typedef IModule* (*CreateModuleFunc)();
        CreateModuleFunc func = (CreateModuleFunc)PlatformAPI()->GetModuleFunctionAddress(hModule, CREATE_MODULE_FUNC_NAME);
        if (func)
        {
            IModule* Module = func();
            MT_ASSERT(Module);
            Module->Init();
            m_Modules.Add(Module);
			m_ModulesMap.Add(ModuleName, Module);
            
            return Module;
        }
        else
        {
            MT_ASSERT(0);
        }
    }

    MT_ASSERT(0);
    return nullptr;
}
