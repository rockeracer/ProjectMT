#pragma once

struct ImDrawData;
union SDL_Event;
class MTWindow;
class MTRDIWindowView;
class IRenderAPI;
struct MTGUIInputState;

class MTimguiController
{
public:
    void Init();
    void Shutdown();
    
    typedef void (*RenderGUICallback)(ImDrawData* data);
    typedef const char* (*GetClipboardTextCallback)(void* user_data);
    typedef void (*SetClipboardTextCallback)(void* user_data, const char* text);
    void SetCallback(RenderGUICallback RenderGUI, GetClipboardTextCallback GetClipboardText, SetClipboardTextCallback SetClipboardText);
    
    void InitFrame(MTWindow* Window, const MTGUIInputState& InputState, float DeltaTime);
    
    void Render(MTRDIWindowView* View);
    
    
    
    void ShowSampleUI();
    
private:
    RenderGUICallback OnRenderGUI = nullptr;
    GetClipboardTextCallback OnGetClipboardText = nullptr;
    SetClipboardTextCallback OnSetClipboardText = nullptr;
};
