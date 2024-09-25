#include "Shader.h"
#include "ShaderModule.h"

extern "C" SHADER_API IShaderModule* CreateModule()
{
    return new MTShaderModule;
}
