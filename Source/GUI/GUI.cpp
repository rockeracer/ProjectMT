#include "GUI.h"
#include "GUIModule.h"

extern "C" GUI_API IGUIModule* CreateModule()
{
    return new MTGUIModule;
}
