#pragma once

#include "Shader.h"

class MTMetalShaderCodeFormat : public IShaderCodeFormat
{
public:
	virtual MTString GetTextureDeclarationCode(MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName) override
	{
		MTString Code;
		char StrBuffer[256];

		sprintf_s(StrBuffer, "texture2d<float, access::sample> %s%d[[texture(%d)]];\n\t",
			TextureName.c_str(), TextureIndex, TextureIndex);
		Code += StrBuffer;

		sprintf_s(StrBuffer, "sampler %s%d[[sampler(%d)]];\n\t",
			SamplerName.c_str(), TextureIndex, TextureIndex);
		Code += StrBuffer;

		return StrBuffer;
	}

	virtual MTString GetSampleTextureCode(const MTString& VariableName, MTInt32 TextureIndex, const MTString& TextureName, const MTString& SamplerName, const MTString& UVInput) override
	{
		// ex) float4 Node0 = Textures.MaterialTexture0.sample(Textures.MaterialSampler0, Param.Texcoord[0]);
		return "float4 " + VariableName + " = " + "Textures" + "." + TextureName + MTString::ToString(TextureIndex) + "." + "sample(Textures." + MTShaderCodeKeyword::SamplerName() + MTString::ToString(TextureIndex) + ", " + UVInput + ");";
	}
};
