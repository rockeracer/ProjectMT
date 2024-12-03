#include "ShaderGraphNodeVisitor.h"
#include "ShaderGraphNode.h"

void MTShaderGraphNodeVisitor::Traverse(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat)
{
	if (Node)
	{
		for (MTInt32 i = 0; i < Node->GetInputPinCount(); ++i)
		{
			Traverse(Node->GetInputPin(i)->GetLinkedNode(0), ShaderCodeFormat);
		}

		Visit(Node, ShaderCodeFormat);
	}
}



void MTShaderGraphNodeResetIndex::Visit(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat)
{
	Node->ResetIndex();
}



void MTShaderCodeGenerator::GenerateCode(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat)
{
	m_Index = 0;
	m_TextureIndex = 0;
	m_GeneratedCode.Clear();

	MTShaderGraphNodeResetIndex().Traverse(Node, ShaderCodeFormat);

	Traverse(Node, ShaderCodeFormat);
}

MTString MTShaderCodeGenerator::GetShaderCode() const
{
	return m_GeneratedCode;
}

MTString MTShaderCodeGenerator::GetTextureDeclarationCode(IShaderCodeFormat* ShaderCodeFormat) const
{
	MTString Code;

	for (MTInt32 i = 0; i < m_TextureIndex; ++i)
	{
		Code += ShaderCodeFormat->GetTextureDeclarationCode(i, MTShaderCodeKeyword::TextureName(), MTShaderCodeKeyword::SamplerName());
	}

	return Code;
}

void MTShaderCodeGenerator::Visit(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat)
{
	if (Node->IsValidIndex() == false)
	{
		Node->SetIndex(m_Index++);

		MTShaderGraphTextureSampleNode* TextureSampleNode = dynamic_cast<MTShaderGraphTextureSampleNode*>(Node);
		if (TextureSampleNode)
		{
			TextureSampleNode->SetTextureIndex(m_TextureIndex++);
		}

		m_GeneratedCode += Node->GetShaderCode(ShaderCodeFormat) + "\n\t";
	}
}
