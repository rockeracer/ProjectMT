#pragma once

#include "Shader.h"

class MTDX12ShaderCodeFormat : public IShaderCodeFormat
{
public:
	virtual MTString GetTextureDeclarationCode(MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName) override
	{
		MTString Code;
		char StrBuffer[256];

		sprintf_s(StrBuffer, "Texture2D %s%d : register(t%d);\n\t",
			TextureName.c_str(), TextureIndex, TextureIndex);
		Code += StrBuffer;

		sprintf_s(StrBuffer, "SamplerState %s%d : register(s%d);\n\t",
			SamplerName.c_str(), TextureIndex, TextureIndex);
		Code += StrBuffer;

		return Code;
	}

	virtual MTString GetSampleTextureCode(const MTString& VariableName, MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName, const MTString& UVInput) override
	{
		// ex) float4 Node0 = MaterialTexture0.Sample(MaterialSampler0, Param.Texcoord[0]);
		return "float4 " + VariableName + " = " + TextureName + MTString::ToString(TextureIndex) + ".Sample(" + SamplerName + MTString::ToString(TextureIndex) + ", " + UVInput + ");";
	}
};
