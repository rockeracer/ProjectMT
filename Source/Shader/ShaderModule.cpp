#include "ShaderModule.h"
#include "ShaderGraphMaterialNode.h"
#include "ShaderGraphNodeVisitor.h"

#include "File.h"

void MTShaderModule::Init()
{
	MTFile VertexShaderTemplateFile("../../Shader/VertexShaderTemplate.shd", MTFile::Read);
	MTFile PixelShaderTemplateFile("../../Shader/PixelShaderTemplate.shd", MTFile::Read);

	m_VertexShaderTemplateCode = VertexShaderTemplateFile.ToString();
	m_PixelShaderTemplateCode = PixelShaderTemplateFile.ToString();

	VertexShaderTemplateFile.Close();
	PixelShaderTemplateFile.Close();
}

void MTShaderModule::Destroy()
{

}

MTShaderCode MTShaderModule::GetMaterialShaderCode(IShaderCodeFormat* ShaderCodeFormat)
{
	MTShaderGraphMaterialNode MaterialNode;
	MTShaderGraphTextureSampleNode TextureNode0, TextureNode1;
	MaterialNode.GetInputPin(0)->LinkWith(TextureNode0.GetOutputPin(0));
	MaterialNode.GetInputPin(7)->LinkWith(TextureNode1.GetOutputPin(0));

	MTShaderCodeGenerator ShaderCodeGenerator;
	ShaderCodeGenerator.GenerateCode(&MaterialNode, ShaderCodeFormat);

	MTShaderCode ShaderCode;
	ShaderCode.VertexShaderCode = ShaderCode.PixelShaderCode = ShaderCodeFormat->GetHeaderCode();

	ShaderCode.VertexShaderCode += m_VertexShaderTemplateCode;
	ShaderCode.VertexShaderCode.Replace("%TextureDeclaration", ShaderCodeGenerator.GetTextureDeclarationCode(ShaderCodeFormat));

	ShaderCode.PixelShaderCode += m_PixelShaderTemplateCode;
	ShaderCode.PixelShaderCode.Replace("%MaterialPixelShaderCode", ShaderCodeGenerator.GetShaderCode());
	ShaderCode.PixelShaderCode.Replace("%TextureDeclaration", ShaderCodeGenerator.GetTextureDeclarationCode(ShaderCodeFormat));

	//    MTFile MaterialShaderFile("../Shader/Material.txt", MTFile::Write);
	//    MaterialShaderFile << ShaderCode;
	//    MaterialShaderFile.Close();

	return ShaderCode;
}
