#include "ShaderGraphNode.h"

void MTShaderGraphNode::InitInput(MTInt32 Count)
{
    for (MTInt32 i = 0; i < Count; ++i)
    {
        m_InputPin.Add(MTGraphPin(this, true));
    }
}
