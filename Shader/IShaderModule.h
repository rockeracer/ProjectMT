#pragma once

#include "IModule.h"

class IShaderCodeFormat;

struct MTShaderCode
{
	MTString VertexShaderCode;
	MTString PixelShaderCode;
};

enum class MTShaderPlatform
{
	DX12,
	VULKAN,
	METAL,
};

class IShaderModule : public IModule
{
public:
	virtual MTShaderCode GetMaterialShaderCode(IShaderCodeFormat* ShaderCodeFormat) = 0;
};
