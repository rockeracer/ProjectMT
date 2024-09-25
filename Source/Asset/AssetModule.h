#pragma once

#include "IModule.h"

class MTAssetModule : public IModule
{
public:
    virtual void Init() override        {}
    virtual void Destroy() override     {}
};
