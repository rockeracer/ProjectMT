#pragma once

#ifdef SHADER_EXPORTS
#define SHADER_API MT_DLL_EXPORT
#else
#define SHADER_API MT_DLL_IMPORT
#endif

#include "IShaderModule.h"

class IShaderCodeFormat
{
public:
	virtual ~IShaderCodeFormat() {}

	virtual MTString GetHeaderCode() { return ""; }

	virtual MTString GetTextureDeclarationCode(MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName) = 0;

	virtual MTString GetSampleTextureCode(const MTString& VariableName, MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName, const MTString& UVInput) = 0;
};
