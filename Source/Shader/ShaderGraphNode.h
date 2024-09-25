#pragma once

class MTShaderCodeKeyword
{
public:
    static MTString TextureName()
    {
        return "MaterialTexture";
    }

	static MTString SamplerName()
    {
        return "MaterialSampler";
    }
};

class MTShaderGraphNode;

class MTGraphPin
{
public:
    MTGraphPin(MTShaderGraphNode* Node, bool bInput)
    : m_Node(Node), m_bInput(bInput)
    {
    }
    
    bool LinkWith(MTGraphPin* Pin)
    {
        if (Pin &&
            (IsLinkedWith(Pin) == false) &&
            (m_bInput != Pin->m_bInput))
        {
            if (m_bInput)
            {
                BreakAllLink();
            }
            
            m_LinkedPin.Add(Pin);
            
            if (Pin->IsLinkedWith(this) == false)
            {
                Pin->LinkWith(this);
            }
            
            return true;
        }
        
        return false;
    }
    
    bool IsLinkedWith(MTGraphPin* Pin) const
    {
        return m_LinkedPin.Contain(Pin);
    }
    
    void BreakLink(MTGraphPin* Pin)
    {
        if (Pin && IsLinkedWith(Pin))
        {
			m_LinkedPin.Remove(Pin);
            
            Pin->BreakLink(this);
        }
    }
    
    void BreakAllLink()
    {
        MTArray<MTGraphPin*> LinkedPin = m_LinkedPin;
        
        for (MTGraphPin* Pin : LinkedPin)
        {
            BreakLink(Pin);
        }
    }
    
    MTShaderGraphNode* GetNode() const
    {
        return m_Node;
    }
    
    MTShaderGraphNode* GetLinkedNode(MTInt32 Index) const
    {
        if (m_LinkedPin.IsValidIndex(Index))
        {
            return m_LinkedPin[Index]->GetNode();
        }
        
        return nullptr;
    }
    
private:
    MTShaderGraphNode* m_Node = nullptr;
    
    MTArray<MTGraphPin*> m_LinkedPin;
    
    bool m_bInput = false;
};

class SHADER_API MTShaderGraphNode
{
public:
    void InitInput(MTInt32 Count);

    void InitOutput(MTInt32 Count)
    {
        for (MTInt32 i = 0; i < Count; ++i)
        {
            m_OutputPin.Add(MTGraphPin(this, false));
        }
    }
    
    MTGraphPin* GetInputPin(MTInt32 Index)
    {
        if (m_InputPin.IsValidIndex(Index))
        {
            return &m_InputPin[Index];
        }
        
        return nullptr;
    }
    
    MTGraphPin* GetOutputPin(MTInt32 Index)
    {
        if (m_OutputPin.IsValidIndex(Index))
        {
            return &m_OutputPin[Index];
        }
        
        return nullptr;
    }
    
    MTInt32 GetInputPinCount() const
    {
        return (MTInt32)m_InputPin.Num();
    }
    
    MTInt32 GetOutputPinCount() const
    {
        return (MTInt32)m_OutputPin.Num();
    }
    
    void SetIndex(MTInt32 Index)
    {
        m_Index = Index;
    }
    
    void ResetIndex()
    {
        SetIndex(INVALID_INDEX);
    }
    
    bool IsValidIndex() const
    {
        return m_Index > INVALID_INDEX;
    }
    
    MTInt32 GetIndex() const
    {
        return m_Index;
    }
    
    virtual MTString GetShaderCode(IShaderCodeFormat* ShaderCodeFormat)
    {
        return "float4 " + GetVariableName() + " = float4(0, 0, 0, 0);";
    }
    
    MTString GetVariableName() const
    {
        return "Node" + MTString::ToString(GetIndex());
    }
    
private:
    static const MTInt32 INVALID_INDEX = -1;
    
    MTArray<MTGraphPin> m_InputPin;
    MTArray<MTGraphPin> m_OutputPin;
    
    MTInt32 m_Index= INVALID_INDEX;
};

class MTShaderGraphMathNode : public MTShaderGraphNode
{
public:
    virtual MTString GetShaderCode(IShaderCodeFormat* ShaderCodeFormat) override
    {
        MTString ShaderCode = "float4 " + GetVariableName() + " = " + m_BaseCode + ";";
        for (MTInt32 i = 0; i < GetInputPinCount(); ++i)
        {
            MTShaderGraphNode* InputNode = GetInputPin(i)->GetLinkedNode(0);
			ShaderCode.Replace("{" + MTString::ToString(i) + "}", InputNode ? InputNode->GetVariableName() : "0" );
        }
        
        return ShaderCode;
    }
    
    void SetBaseCode(const MTString& Code)
    {
        m_BaseCode = Code;
    }
    
private:
    MTString m_BaseCode;
};

class MTShaderGraphTextureSampleNode : public MTShaderGraphNode
{
public:
    MTShaderGraphTextureSampleNode()
    {
        InitInput(1);
        InitOutput(1);
    }
    
    virtual MTString GetShaderCode(IShaderCodeFormat* ShaderCodeFormat) override
    {
        MTShaderGraphNode* InputNode = GetInputPin(0)->GetLinkedNode(0);
        MTString UVInput = InputNode ? InputNode->GetVariableName() : "Param.Texcoord[0]";

		return ShaderCodeFormat->GetSampleTextureCode(GetVariableName(), m_TextureIndex, MTShaderCodeKeyword::TextureName(), MTShaderCodeKeyword::SamplerName(), UVInput);
    }
        
    void SetTextureIndex(MTInt32 Index)
    {
        m_TextureIndex = Index;
    }
        
private:
    MTInt32 m_TextureIndex = 0;
};
