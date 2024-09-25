#pragma once

#include "ICoreObjectModule.h"

class MTCoreObjectModule : public ICoreObjectModule
{
public:
    virtual void Init() override;
    virtual void Destroy() override;
};
