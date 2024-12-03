#include "Core.h"
#include "GUI.h"

int main(int argc, const char * argv[])
{
    InitCore();
    
    Core::LoadModule("Engine");
    
    IGUIModule* GUI = Core::LoadModule<IGUIModule>("GUI");
    
    if (GUI == nullptr)
    {
        return -1;
    }

    MTWindow* window = GUI->CreateWindow();
    
    bool bDone = false;
    MTClock Clock;
    while (!bDone)
    {
        Clock.Update();
        
        bDone = GUI->ProcessEvent();
        
        if (bDone)
        {
            GUI->ExitRender();
        }
        else
        {
            GUI->RenderWindow(window, Clock.GetDeltaSeconds());
        }
    }
    
    GUI->DestroyWindow(window);
    
    ShutdownCore();
    
    return 0;
}
