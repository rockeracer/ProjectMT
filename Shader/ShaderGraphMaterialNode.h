#pragma once

#include "ShaderGraphNode.h"

class SHADER_API MTShaderGraphMaterialNode : public MTShaderGraphNode
{
public:
    MTShaderGraphMaterialNode();
    
    virtual MTString GetShaderCode(IShaderCodeFormat* ShaderCodeFormat) override;
    
private:
    MTString GetComponentAccessor(MTInt32 Count) const;
};
