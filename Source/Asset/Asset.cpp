#include "Asset.h"
#include "AssetModule.h"

extern "C" ASSET_API IModule* CreateModule()
{
    return new MTAssetModule;
}
