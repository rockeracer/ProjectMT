#include "GUIModule.h"
#include "Window.h"
#include "GUIRenderer.h"

void MTGUIModule::Init()
{
#ifdef _WIN32
	MTString DLLPath = "../ThirdParty/SDL2/lib/x64/SDL2.dll";
	SDL_DLLHandle = PlatformAPI()->LoadModule(DLLPath.c_str());
	MT_ASSERT(SDL_DLLHandle != nullptr);
#endif

    if (SDL_Init(SDL_INIT_TIMER) != 0)
    {
        MT_LOG("Error : %s/n", SDL_GetError());
        return;
    }
    
    m_Renderer = new MTGUIRenderer();

    return;
}

void MTGUIModule::Destroy()
{
    MT_ASSERT(m_Renderer == nullptr);
    
    SDL_Quit();

	if (SDL_DLLHandle)
	{
		PlatformAPI()->DestroyModule(SDL_DLLHandle);
	}
}

MTWindow* MTGUIModule::CreateWindow()
{
    MTWindow* NewWindow = new MTWindow;
	m_Windows.Add(NewWindow);
    return NewWindow;
}

void MTGUIModule::DestroyWindow(MTWindow* Window)
{
    if (Window)
    {
        delete Window;
        
		m_Windows.Remove(Window);
    }
}

bool MTGUIModule::ProcessEvent()
{
    bool bQuit = false;
    
    m_InputState.InitLoop();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ProcessEvent(event);
        
        if (event.type == SDL_QUIT)
        {
            bQuit = true;
        }
    }
    
    MTUInt32 MouseMask = SDL_GetMouseState(&m_InputState.MousePosX, &m_InputState.MousePosY);
    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    m_InputState.bMousePressed[0] |= (MouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    m_InputState.bMousePressed[1] |= (MouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    m_InputState.bMousePressed[2] |= (MouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    
    return bQuit;
}

bool MTGUIModule::ProcessEvent(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_WINDOWEVENT:
        {
            switch (event.window.event)
            {
                case SDL_WINDOWEVENT_RESIZED:
                {
                    for (MTWindow* Window : m_Windows)
                    {
                        if (Window->GetWindowID() == event.window.windowID)
                        {
                            Window->Resize(event.window.data1, event.window.data2);
                        }
                    }

                    break;
                }
            }
            return true;
        }
        case SDL_MOUSEWHEEL:
        {
            if (event.wheel.y > 0)
            {
                m_InputState.MouseWheel = 1;
            }
            if (event.wheel.y < 0)
            {
                m_InputState.MouseWheel = -1;
            }
            return true;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                m_InputState.bMousePressed[0] = true;
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                m_InputState.bMousePressed[1] = true;
            }
            if (event.button.button == SDL_BUTTON_MIDDLE)
            {
                m_InputState.bMousePressed[2] = true;
            }
            return true;
        }
        case SDL_TEXTINPUT:
        {
            m_InputState.Text.Add(event.text.text);
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            MTInt32 key = event.key.keysym.sym & ~SDLK_SCANCODE_MASK;
            m_InputState.bKeyPressed[key] = (event.type == SDL_KEYDOWN);
            m_InputState.bKeyShiftPressed = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            m_InputState.bKeyCtrlPressed = ((SDL_GetModState() & KMOD_CTRL) != 0);
            m_InputState.bKeyAltPressed = ((SDL_GetModState() & KMOD_ALT) != 0);
            m_InputState.bKeySuperPressed = ((SDL_GetModState() & KMOD_GUI) != 0);
            return true;
        }
    }
    return false;
}

void MTGUIModule::RenderWindow(MTWindow* Window, float DeltaTime)
{
    if (Window == nullptr)
    {
        return;
    }

    if (m_Renderer)
    {
        m_Renderer->Render(Window, m_InputState, DeltaTime);
    }
}

void MTGUIModule::ExitRender()
{
    if (m_Renderer)
    {
        delete m_Renderer;
        m_Renderer = nullptr;
    }
}
