#pragma once

#include "IEngineModule.h"

class MTEngineModule : public IEngineModule
{
public:
    virtual void Init() override;
    virtual void Destroy() override {}
};

