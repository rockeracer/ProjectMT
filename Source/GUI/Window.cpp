#include "Window.h"
#include "RenderCore.h"

MTWindow::MTWindow()
{
    m_Window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);
    
	IRenderCoreModule* RenderCoreModule = Core::LoadModule<IRenderCoreModule>("RenderCore");
    if (RenderCoreModule && m_Window)
    {
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        
        if(SDL_GetWindowWMInfo(m_Window, &info))
        {
            void* Handle = nullptr;
#ifdef _WIN32
			Handle = info.info.win.window;
#elif __APPLE__
            Handle = info.info.cocoa.window;
#endif
			MTUInt32 Widght, Height;
			GetDisplaySize(Widght, Height);

            m_RenderView = RenderCoreModule->CreateWindowView(Handle, Widght, Height);
        }
    }
}

MTWindow::~MTWindow()
{
    if (m_Window)
    {
        if (m_RenderView)
        {
            delete m_RenderView;
        }

		SDL_DestroyWindow(m_Window);
    }
}

void MTWindow::GetWindowSize(MTUInt32& outWidth, MTUInt32& outHeight)
{
	if (m_Window)
	{
		MTInt32 Width, Height;
		SDL_GetWindowSize(m_Window, &Width, &Height);

		outWidth = Width;
		outHeight = Height;
	}
}

void MTWindow::GetDisplaySize(MTUInt32& outWidth, MTUInt32& outHeight)
{
	if (m_Window)
	{
		MTInt32 Width, Height;
		SDL_GL_GetDrawableSize(m_Window, &Width, &Height);

		outWidth = Width;
		outHeight = Height;
	}
}

bool MTWindow::HasMouseFocus() const
{
    SDL_GetWindowID(m_Window);
    return SDL_GetWindowFlags(m_Window) & SDL_WINDOW_MOUSE_FOCUS;
}

MTRDIWindowView* MTWindow::GetRenderView() const
{
    return m_RenderView;
}

MTUInt32 MTWindow::GetWindowID() const
{
    return SDL_GetWindowID(m_Window);
}

void MTWindow::Resize(MTInt32 Width, MTInt32 Height)
{
    if (m_RenderView)
    {
		MTUInt32 Widght, Height;
		GetDisplaySize(Widght, Height);
		
		m_RenderView->Resize(Widght, Height);
    }
}
