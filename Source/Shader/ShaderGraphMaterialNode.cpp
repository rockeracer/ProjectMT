#include "ShaderGraphMaterialNode.h"

enum class EMaterialNodeInput
{
    BaseColor = 0,
    Metallic,
    Specular,
    Roughness,
    EmissiveColor,
    Opacity,
    OpacityMask,
    Normal,
    WorldPositionOffset,
    Max
};

struct MTMaterialInputParam
{
    MTString Name;
    MTInt32 ComponentCount;
};

const MTMaterialInputParam MaterialInputParam[] =
{
    {"BaseColor", 3},
    {"Metallic", 1},
    {"Specular", 1},
    {"Roughness", 1},
    {"EmissiveColor", 3},
    {"Opacity", 1},
    {"OpacityMask", 1},
    {"Normal", 3},
    {"WorldPositionOffset", 3},
};

MTShaderGraphMaterialNode::MTShaderGraphMaterialNode()
{
    InitInput((MTInt32)EMaterialNodeInput::Max);
}

MTString MTShaderGraphMaterialNode::GetShaderCode(IShaderCodeFormat* ShaderCodeFormat)
{
    MTString ShaderCode;
    for (MTInt32 i = 0; i < GetInputPinCount(); ++i)
    {
        MTShaderGraphNode* InputNode = GetInputPin(i)->GetLinkedNode(0);
        if (InputNode)
        {
            if (i > 0)
            {
                ShaderCode += "\n\t";
            }
            
            MTString Param = "Param." + MaterialInputParam[i].Name;
            
            MTString InputNodeVariable = InputNode->GetVariableName() + GetComponentAccessor(MaterialInputParam[i].ComponentCount);
            
            if (i == (MTInt32)EMaterialNodeInput::Normal)
            {
                ShaderCode += Param + " = GetPixelShaderParamNormal(" + InputNodeVariable + ", PixelShaderInput);";
            }
            else
            {
                ShaderCode += Param + " = " + InputNodeVariable + ";";
            }
        }
    }
    
    return ShaderCode;
}

MTString MTShaderGraphMaterialNode::GetComponentAccessor(MTInt32 Count) const
{
    switch(Count)
    {
        case 1: return ".x";
        case 2: return ".xy";
        case 3: return ".xyz";
        case 4: return ".xyzw";
    }
    
    return "";
}
