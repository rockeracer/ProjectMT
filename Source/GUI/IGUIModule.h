#pragma once

#include "IModule.h"

#undef CreateWindow

class MTWindow;

class IGUIModule : public IModule
{
public:
    IGUIModule()            {}
    virtual ~IGUIModule()   {}
    
    virtual MTWindow* CreateWindow() = 0;
    virtual void DestroyWindow(MTWindow* Window) = 0;
    
    virtual bool ProcessEvent() = 0;
    
    virtual void RenderWindow(MTWindow* Window, float DeltaTime) = 0;
    
    virtual void ExitRender() = 0;
};
