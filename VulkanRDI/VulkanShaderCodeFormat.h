#pragma once

#include "Shader.h"

class MTVulkanShaderCodeFormat : public IShaderCodeFormat
{
public:
	virtual MTString GetHeaderCode() override
	{
		return "#version 450\n"
			"#extension GL_ARB_separate_shader_objects : enable\n";
	}

	virtual MTString GetTextureDeclarationCode(MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName) override
	{
		char StrBuffer[256];

		sprintf_s(StrBuffer, "layout(binding = %d) uniform sampler2D %s%d;\n\t", 
			TextureIndex + 1, TextureName.c_str(), TextureIndex);

		return StrBuffer;
	}

	virtual MTString GetSampleTextureCode(const MTString& VariableName, MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName, const MTString& UVInput) override
	{
		// ex) float4 Node0 = texture(MaterialTexture0, Param.Texcoord[0]);
		return "float4 " + VariableName + " = " + "texture(" + TextureName + MTString::ToString(TextureIndex) + ", " + UVInput + ");";
	}
};
