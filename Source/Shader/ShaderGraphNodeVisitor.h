#pragma once

class MTShaderGraphNode;

class SHADER_API MTShaderGraphNodeVisitor
{
public:
    void Traverse(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat);
    
private:
    virtual void Visit(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat) = 0;
};

class MTShaderGraphNodeResetIndex : public MTShaderGraphNodeVisitor
{
private:
    virtual void Visit(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat) override;
};


class SHADER_API MTShaderCodeGenerator : public MTShaderGraphNodeVisitor
{
public:
	void GenerateCode(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat);

    MTString GetShaderCode() const;

    MTString GetTextureDeclarationCode(IShaderCodeFormat* ShaderCodeFormat) const;

private:
    virtual void Visit(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat) override;

    MTInt32 m_Index = 0;
    MTInt32 m_TextureIndex = 0;
    MTString m_GeneratedCode;
};
