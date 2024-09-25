#pragma once

#include "IShaderModule.h"

class MTShaderModule : public IShaderModule
{
public:
    virtual void Init() override;
    virtual void Destroy() override;
 
    virtual MTShaderCode GetMaterialShaderCode(IShaderCodeFormat* ShaderCodeFormat) override;
    
private:
    MTString m_VertexShaderTemplateCode;
	MTString m_PixelShaderTemplateCode;
};
