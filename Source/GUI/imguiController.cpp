#include "imguiController.h"
#include "RenderCore.h"
#include "InputState.h"

#include "imgui.h"

static MTRDIWindowView* g_CurrentRenderView = nullptr;
 
void ImGui_RenderDrawLists(ImDrawData* DrawData)
{
    IRenderCoreModule* RenderCore = Core::LoadModule<IRenderCoreModule>("RenderCore");
    if (RenderCore)
    {
        ImGuiIO& io = ImGui::GetIO();
        
        DrawData->ScaleClipRects(io.DisplayFramebufferScale);

		RenderCore->RenderGUI(g_CurrentRenderView, DrawData);
    }
}

void MTimguiController::Init()
{
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;
    
    io.RenderDrawListsFn = ImGui_RenderDrawLists;
    io.SetClipboardTextFn = OnSetClipboardText;
    io.GetClipboardTextFn = OnGetClipboardText;
    io.ClipboardUserData = nullptr;
    
    IRenderCoreModule* RenderCore = Core::LoadModule<IRenderCoreModule>("RenderCore");
    if (RenderCore)
    {
        MTGUIFontTextureDesc FontTextureDesc;
        io.Fonts->GetTexDataAsRGBA32(&FontTextureDesc.Pixels, &FontTextureDesc.Width, &FontTextureDesc.Height);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
        MTGUIVertexDesc VertexDesc;
        VertexDesc.PosOffset = OFFSETOF(ImDrawVert, pos);
        VertexDesc.UVOffset = OFFSETOF(ImDrawVert, uv);
        VertexDesc.ColorOffset = OFFSETOF(ImDrawVert, col);
        VertexDesc.VertexSize = sizeof(ImDrawVert);
#undef OFFSETOF
        
        io.Fonts->TexID = RenderCore->InitGUIRender(FontTextureDesc, VertexDesc);
    }
}

void MTimguiController::Shutdown()
{
	ImGui::DestroyContext();
}

void MTimguiController::SetCallback(RenderGUICallback RenderGUI, GetClipboardTextCallback GetClipboardText, SetClipboardTextCallback SetClipboardText)
{
    OnRenderGUI = RenderGUI;
    OnGetClipboardText = GetClipboardText;
    OnSetClipboardText = SetClipboardText;
}

void MTimguiController::InitFrame(MTWindow* Window, const MTGUIInputState& InputState, float DeltaTime)
{
    if (Window == nullptr)
    {
        return;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Setup display size (every frame to accommodate for window resizing)
	MTUInt32 WindowWidth, WindowHeight;
	Window->GetWindowSize(WindowWidth, WindowHeight);
	MTUInt32 DisplayWidth, DisplayHeight;
	Window->GetDisplaySize(DisplayWidth, DisplayHeight);

    io.DisplaySize = ImVec2((float)WindowWidth, (float)WindowHeight);
    io.DisplayFramebufferScale = ImVec2(WindowWidth > 0 ? ((float)DisplayWidth / WindowWidth) : 0, WindowHeight > 0 ? ((float)DisplayHeight / WindowHeight) : 0);
    
    // Setup time step
    io.DeltaTime = DeltaTime > 0.0 ? DeltaTime : (float)(1.0f / 60.0f);
    
    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
    if (Window->HasMouseFocus())
    {
        io.MousePos = ImVec2((float)InputState.MousePosX, (float)InputState.MousePosY);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    }
    else
    {
        io.MousePos = ImVec2(-1, -1);
    }
    
    io.MouseDown[0] = InputState.bMousePressed[0];
    io.MouseDown[1] = InputState.bMousePressed[1];
    io.MouseDown[2] = InputState.bMousePressed[2];
    
    io.MouseWheel = InputState.MouseWheel;
    
    // Hide OS mouse cursor if ImGui is drawing it
    SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);
    

    // Start the frame
    ImGui::NewFrame();
}

void MTimguiController::Render(MTRDIWindowView* View)
{
    g_CurrentRenderView = View;
    ImGui::Render();
}





void MTimguiController::ShowSampleUI()
{
    static bool show_test_window = true;
    static bool show_another_window = false;
    
    static ImVec4 clear_color = ImColor(114, 144, 154);
    
    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        if (ImGui::Button("Test Window")) show_test_window ^= 1;
        if (ImGui::Button("Another Window")) show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
    
    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window)
    {
        ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello");
        ImGui::End();
    }
    
    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowDemoWindow(&show_test_window);
    }
}
