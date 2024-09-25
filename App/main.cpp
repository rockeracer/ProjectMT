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
    
    bool done = false;
    MTClock Clock;
    while (!done)
    {
        Clock.Update();
        
        done = GUI->ProcessEvent();
        
        if (done)
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
