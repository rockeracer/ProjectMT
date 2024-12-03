#include "Core.h"
#include "ModuleManager.h"

#ifdef _WIN32
#include "Windows/WindowsAPI.h"
MTWindowsAPI g_PlatformAPI;
#elif __APPLE__
#include "Mac/MacAPI.h"
MTMacAPI g_PlatformAPI;
#endif

bool g_bInitCore = false;
IModuleManager* g_ModuleManager = nullptr;

void InitCore()
{
	if (g_bInitCore == false)
	{
		g_ModuleManager = new MTModuleManager;

		g_bInitCore = true;
	}
}

void ShutdownCore()
{
	if (g_bInitCore)
	{
		if (g_ModuleManager)
		{
			g_ModuleManager->Destroy();
			delete g_ModuleManager;
			g_ModuleManager = nullptr;
		}
	}
}

IPlatformAPI* PlatformAPI()
{
	return &g_PlatformAPI;
}

IModuleManager* ModuleManager()
{
	return g_ModuleManager;
}
