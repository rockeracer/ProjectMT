#include "gtest/gtest.h"
#include "Core.h"

#include "Shader.h"
#include "ShaderGraphNode.h"
#include "ShaderGraphNodeVisitor.h"
#include "ShaderGraphMaterialNode.h"

#include "TestRDIUtils.h"

TEST(ShaderGraphNodeTest, InitInputPin)
{
	MTShaderGraphNode Node0, Node;
	Node0.InitInput(0);
	Node.InitInput(3);

	EXPECT_EQ(0, Node0.GetInputPinCount());
	EXPECT_EQ(nullptr, Node0.GetInputPin(0));

	EXPECT_EQ(3, Node.GetInputPinCount());
	EXPECT_NE(nullptr, Node.GetInputPin(0));
	EXPECT_NE(nullptr, Node.GetInputPin(1));
	EXPECT_NE(nullptr, Node.GetInputPin(2));
	EXPECT_EQ(nullptr, Node.GetInputPin(3));
	EXPECT_NE(Node.GetInputPin(0), Node.GetInputPin(1));
	EXPECT_NE(Node.GetInputPin(1), Node.GetInputPin(2));
	EXPECT_NE(Node.GetInputPin(2), Node.GetInputPin(0));
}

TEST(ShaderGraphNodeTest, InitOutputPin)
{
	MTShaderGraphNode Node0, Node;
	Node0.InitInput(0);
	Node.InitOutput(3);

	EXPECT_EQ(0, Node0.GetOutputPinCount());
	EXPECT_EQ(nullptr, Node0.GetOutputPin(0));

	EXPECT_EQ(3, Node.GetOutputPinCount());
	EXPECT_NE(nullptr, Node.GetOutputPin(0));
	EXPECT_NE(nullptr, Node.GetOutputPin(1));
	EXPECT_NE(nullptr, Node.GetOutputPin(2));
	EXPECT_EQ(nullptr, Node.GetOutputPin(3));
	EXPECT_NE(Node.GetOutputPin(0), Node.GetOutputPin(1));
	EXPECT_NE(Node.GetOutputPin(1), Node.GetOutputPin(2));
	EXPECT_NE(Node.GetOutputPin(2), Node.GetOutputPin(0));
}

TEST(ShaderGraphNodeTest, Link)
{
	MTShaderGraphNode NodeA, NodeB;
	NodeA.InitInput(1);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);

	ASSERT_NE(nullptr, NodeA.GetOutputPin(0));
	ASSERT_NE(nullptr, NodeB.GetInputPin(0));

	EXPECT_TRUE(NodeB.GetInputPin(0)->LinkWith(NodeA.GetOutputPin(0)));
	EXPECT_TRUE(NodeB.GetInputPin(0)->IsLinkedWith(NodeA.GetOutputPin(0)));
	EXPECT_TRUE(NodeA.GetOutputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));
}

TEST(ShaderGraphNodeTest, LinkInvalidInput)
{
	MTShaderGraphNode Node;
	Node.InitInput(1);
	Node.InitOutput(1);

	ASSERT_NE(nullptr, Node.GetOutputPin(0));
	ASSERT_NE(nullptr, Node.GetInputPin(0));

	EXPECT_FALSE(Node.GetInputPin(0)->LinkWith(nullptr));
	EXPECT_FALSE(Node.GetOutputPin(0)->IsLinkedWith(nullptr));
}

TEST(ShaderGraphNodeTest, LinkWrongDirection)
{
	MTShaderGraphNode NodeA, NodeB;
	NodeA.InitInput(1);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);

	ASSERT_NE(nullptr, NodeA.GetOutputPin(0));
	ASSERT_NE(nullptr, NodeB.GetInputPin(0));

	EXPECT_FALSE(NodeB.GetInputPin(0)->LinkWith(NodeA.GetInputPin(0)));
	EXPECT_FALSE(NodeB.GetInputPin(0)->IsLinkedWith(NodeA.GetInputPin(0)));
	EXPECT_FALSE(NodeA.GetInputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));

	EXPECT_FALSE(NodeB.GetOutputPin(0)->LinkWith(NodeA.GetOutputPin(0)));
	EXPECT_FALSE(NodeB.GetOutputPin(0)->IsLinkedWith(NodeA.GetOutputPin(0)));
	EXPECT_FALSE(NodeA.GetOutputPin(0)->IsLinkedWith(NodeB.GetOutputPin(0)));
}

TEST(ShaderGraphNodeTest, OutputPinMultiLink)
{
	MTShaderGraphNode NodeA, NodeB, NodeC;
	NodeA.InitInput(1);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);
	NodeC.InitInput(1);
	NodeC.InitOutput(1);

	EXPECT_TRUE(NodeA.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0)));
	EXPECT_TRUE(NodeB.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0)));

	EXPECT_TRUE(NodeA.GetInputPin(0)->IsLinkedWith(NodeC.GetOutputPin(0)));
	EXPECT_TRUE(NodeB.GetInputPin(0)->IsLinkedWith(NodeC.GetOutputPin(0)));
	EXPECT_TRUE(NodeC.GetOutputPin(0)->IsLinkedWith(NodeA.GetInputPin(0)));
	EXPECT_TRUE(NodeC.GetOutputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));
}

TEST(ShaderGraphNodeTest, BreakLink)
{
	MTShaderGraphNode NodeA, NodeB, NodeC;
	NodeA.InitInput(1);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);
	NodeC.InitInput(1);
	NodeC.InitOutput(1);

	NodeB.GetInputPin(0)->LinkWith(NodeA.GetOutputPin(0));
	NodeC.GetInputPin(0)->LinkWith(NodeB.GetOutputPin(0));

	NodeB.GetInputPin(0)->BreakLink(NodeA.GetOutputPin(0));
	NodeB.GetOutputPin(0)->BreakLink(NodeC.GetInputPin(0));

	EXPECT_FALSE(NodeB.GetInputPin(0)->IsLinkedWith(NodeA.GetOutputPin(0)));
	EXPECT_FALSE(NodeA.GetOutputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));
	EXPECT_FALSE(NodeB.GetOutputPin(0)->IsLinkedWith(NodeC.GetInputPin(0)));
	EXPECT_FALSE(NodeC.GetInputPin(0)->IsLinkedWith(NodeB.GetOutputPin(0)));
}

TEST(ShaderGraphNodeTest, BreakLinkInvalidInput)
{
	MTShaderGraphNode NodeA, NodeB;
	NodeA.InitInput(1);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);

	NodeB.GetInputPin(0)->LinkWith(NodeA.GetOutputPin(0));

	NodeB.GetInputPin(0)->BreakLink(nullptr);
	NodeB.GetOutputPin(0)->BreakLink(nullptr);
}

TEST(ShaderGraphNodeTest, BreakAllLink)
{
	MTShaderGraphNode NodeA, NodeB, NodeC;
	NodeA.InitInput(1);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);
	NodeC.InitInput(1);
	NodeC.InitOutput(1);

	NodeA.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0));
	NodeB.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0));


	NodeC.GetOutputPin(0)->BreakAllLink();

	EXPECT_FALSE(NodeA.GetInputPin(0)->IsLinkedWith(NodeC.GetOutputPin(0)));
	EXPECT_FALSE(NodeB.GetInputPin(0)->IsLinkedWith(NodeC.GetOutputPin(0)));
	EXPECT_FALSE(NodeC.GetOutputPin(0)->IsLinkedWith(NodeA.GetInputPin(0)));
	EXPECT_FALSE(NodeC.GetOutputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));


	NodeA.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0));
	NodeB.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0));

	NodeA.GetInputPin(0)->BreakAllLink();

	EXPECT_FALSE(NodeA.GetInputPin(0)->IsLinkedWith(NodeC.GetOutputPin(0)));
	EXPECT_TRUE(NodeB.GetInputPin(0)->IsLinkedWith(NodeC.GetOutputPin(0)));
	EXPECT_FALSE(NodeC.GetOutputPin(0)->IsLinkedWith(NodeA.GetInputPin(0)));
	EXPECT_TRUE(NodeC.GetOutputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));
}

TEST(ShaderGraphNodeTest, InputNewLink)
{
	MTShaderGraphNode NodeA, NodeB, NodeC;
	NodeA.InitInput(1);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);
	NodeC.InitInput(1);
	NodeC.InitOutput(1);

	NodeB.GetInputPin(0)->LinkWith(NodeA.GetOutputPin(0));
	NodeB.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0));

	EXPECT_FALSE(NodeB.GetInputPin(0)->IsLinkedWith(NodeA.GetOutputPin(0)));
	EXPECT_FALSE(NodeA.GetOutputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));
	EXPECT_TRUE(NodeB.GetInputPin(0)->IsLinkedWith(NodeC.GetOutputPin(0)));
	EXPECT_TRUE(NodeC.GetOutputPin(0)->IsLinkedWith(NodeB.GetInputPin(0)));
}

TEST(ShaderGraphNodeTest, GetLinkedNode)
{
	MTShaderGraphNode NodeA, NodeB;
	NodeA.InitInput(2);
	NodeA.InitOutput(1);
	NodeB.InitInput(1);
	NodeB.InitOutput(1);

	NodeA.GetInputPin(0)->LinkWith(NodeB.GetOutputPin(0));

	EXPECT_EQ(&NodeB, NodeA.GetInputPin(0)->GetLinkedNode(0));
	EXPECT_EQ(nullptr, NodeA.GetInputPin(0)->GetLinkedNode(1));
	EXPECT_EQ(nullptr, NodeA.GetInputPin(1)->GetLinkedNode(0));
}

TEST(ShaderGraphNodeVisitorTest, VisitOrder)
{
	MTShaderGraphNode NodeA, NodeB, NodeC, NodeD, NodeE;
	NodeA.InitInput(2);
	NodeA.InitOutput(1);
	NodeA.SetIndex(0);

	NodeB.InitInput(2);
	NodeB.InitOutput(1);
	NodeB.SetIndex(1);

	NodeC.InitInput(1);
	NodeC.InitOutput(1);
	NodeC.SetIndex(2);

	NodeD.InitInput(1);
	NodeD.InitOutput(1);
	NodeD.SetIndex(3);

	NodeE.InitInput(1);
	NodeE.InitOutput(1);
	NodeE.SetIndex(4);

	NodeA.GetInputPin(0)->LinkWith(NodeB.GetOutputPin(0));
	NodeA.GetInputPin(1)->LinkWith(NodeC.GetOutputPin(0));
	NodeB.GetInputPin(0)->LinkWith(NodeD.GetOutputPin(0));
	NodeB.GetInputPin(1)->LinkWith(NodeE.GetOutputPin(0));
	NodeC.GetInputPin(0)->LinkWith(NodeE.GetOutputPin(0));

	class MTShaderGraphNodeOrderCheckVisitor : public MTShaderGraphNodeVisitor
	{
	public:
		virtual void Visit(MTShaderGraphNode* Node, IShaderCodeFormat* ShaderCodeFormat) override
		{
			m_VisitNodeIndex.Add(Node->GetIndex());
		}

		MTInt32 GetVisitNodeIndex(MTInt32 Index)
		{
			if (m_VisitNodeIndex.IsValidIndex(Index))
			{
				return m_VisitNodeIndex[Index];
			}

			return -1;
		}

	private:
		MTArray<MTInt32> m_VisitNodeIndex;
	};
	MTShaderGraphNodeOrderCheckVisitor Visitor;
	Visitor.Traverse(&NodeA, TestRDIUtils::GetDX12ShaderFormat());

	EXPECT_EQ(3, Visitor.GetVisitNodeIndex(0));
	EXPECT_EQ(4, Visitor.GetVisitNodeIndex(1));
	EXPECT_EQ(1, Visitor.GetVisitNodeIndex(2));
	EXPECT_EQ(4, Visitor.GetVisitNodeIndex(3));
	EXPECT_EQ(2, Visitor.GetVisitNodeIndex(4));
	EXPECT_EQ(0, Visitor.GetVisitNodeIndex(5));
}

TEST(ShaderCodeGeneratorTest, GenerateShaderCode)
{
	MTShaderGraphMathNode NodeA, NodeB, NodeC;
	MTShaderGraphNode NodeD, NodeE;
	NodeA.InitInput(2);
	NodeA.InitOutput(1);
	NodeA.SetBaseCode("{0} + {1}");

	NodeB.InitInput(2);
	NodeB.InitOutput(1);
	NodeB.SetBaseCode("{0} - {1}");

	NodeC.InitInput(2);
	NodeC.InitOutput(1);
	NodeC.SetBaseCode("{0} * {1}");

	NodeD.InitInput(1);
	NodeD.InitOutput(1);

	NodeE.InitInput(1);
	NodeE.InitOutput(1);

	NodeA.GetInputPin(0)->LinkWith(NodeB.GetOutputPin(0));
	NodeA.GetInputPin(1)->LinkWith(NodeC.GetOutputPin(0));
	NodeB.GetInputPin(0)->LinkWith(NodeD.GetOutputPin(0));
	NodeB.GetInputPin(1)->LinkWith(NodeE.GetOutputPin(0));
	NodeC.GetInputPin(0)->LinkWith(NodeE.GetOutputPin(0));

	MTShaderCodeGenerator CodeGenerator;
	CodeGenerator.GenerateCode(&NodeA, TestRDIUtils::GetDX12ShaderFormat());

	MTString ExpectCode = "float4 Node0 = float4(0, 0, 0, 0);\n\t";
	ExpectCode += "float4 Node1 = float4(0, 0, 0, 0);\n\t";
	ExpectCode += "float4 Node2 = Node0 - Node1;\n\t";
	ExpectCode += "float4 Node3 = Node1 * 0;\n\t";
	ExpectCode += "float4 Node4 = Node2 + Node3;\n\t";

	EXPECT_EQ(ExpectCode, CodeGenerator.GetShaderCode());
}

TEST(ShaderCodeGeneratorTest, RegenerateShaderCode)
{
	MTShaderGraphMathNode NodeA, NodeB;
	MTShaderGraphNode NodeC, NodeD;
	NodeA.InitInput(2);
	NodeA.InitOutput(1);
	NodeA.SetBaseCode("{0} + {1}");

	NodeB.InitInput(2);
	NodeB.InitOutput(1);
	NodeB.SetBaseCode("{0} - {1}");

	NodeC.InitOutput(1);

	NodeD.InitOutput(1);

	NodeA.GetInputPin(0)->LinkWith(NodeB.GetOutputPin(0));
	NodeA.GetInputPin(1)->LinkWith(NodeC.GetOutputPin(0));

	MTShaderCodeGenerator CodeGenerator;
	CodeGenerator.GenerateCode(&NodeA, TestRDIUtils::GetDX12ShaderFormat());

	NodeB.GetInputPin(0)->LinkWith(NodeD.GetOutputPin(0));

	CodeGenerator.GenerateCode(&NodeA, TestRDIUtils::GetDX12ShaderFormat());

	MTString ExpectCode = "float4 Node0 = float4(0, 0, 0, 0);\n\t";
	ExpectCode += "float4 Node1 = Node0 - 0;\n\t";
	ExpectCode += "float4 Node2 = float4(0, 0, 0, 0);\n\t";
	ExpectCode += "float4 Node3 = Node1 + Node2;\n\t";

	EXPECT_EQ(ExpectCode, CodeGenerator.GetShaderCode());
}

TEST(ShaderGraphTextureSampleNodeTest, TextureSampleNode)
{
	MTShaderGraphTextureSampleNode NodeA, NodeB;
	MTShaderGraphNode NodeC;
	MTShaderGraphMathNode NodeD;
	NodeC.InitOutput(1);
	NodeD.InitInput(2);
	NodeD.InitOutput(1);
	NodeD.SetBaseCode("{0} + {1}");

	NodeB.GetInputPin(0)->LinkWith(NodeC.GetOutputPin(0));
	NodeD.GetInputPin(0)->LinkWith(NodeA.GetOutputPin(0));
	NodeD.GetInputPin(1)->LinkWith(NodeB.GetOutputPin(0));

	MTShaderCodeGenerator CodeGenerator;
	CodeGenerator.GenerateCode(&NodeD, TestRDIUtils::GetDX12ShaderFormat());

	MTString ExpectCode = "float4 Node0 = Textures.MaterialTexture0.sample(Textures.MaterialSampler0, Param.Texcoord[0]);\n\t";
	ExpectCode += "float4 Node1 = float4(0, 0, 0, 0);\n\t";
	ExpectCode += "float4 Node2 = Textures.MaterialTexture1.sample(Textures.MaterialSampler1, Node1);\n\t";
	ExpectCode += "float4 Node3 = Node0 + Node2;\n\t";

	EXPECT_EQ(ExpectCode, CodeGenerator.GetShaderCode());



	NodeD.GetInputPin(0)->LinkWith(NodeB.GetOutputPin(0));
	NodeD.GetInputPin(1)->LinkWith(NodeA.GetOutputPin(0));

	CodeGenerator.GenerateCode(&NodeD, TestRDIUtils::GetDX12ShaderFormat());

	ExpectCode = "float4 Node0 = float4(0, 0, 0, 0);\n\t";
	ExpectCode += "float4 Node1 = Textures.MaterialTexture0.sample(Textures.MaterialSampler0, Node0);\n\t";
	ExpectCode += "float4 Node2 = Textures.MaterialTexture1.sample(Textures.MaterialSampler1, Param.Texcoord[0]);\n\t";
	ExpectCode += "float4 Node3 = Node1 + Node2;\n\t";

	EXPECT_EQ(ExpectCode, CodeGenerator.GetShaderCode());
}

TEST(ShaderCodeGeneratorTest, TextureDeclareCode)
{
	MTShaderGraphMathNode NodeA;
	MTShaderGraphTextureSampleNode NodeB, NodeC;
	NodeA.InitInput(2);
	NodeA.InitOutput(1);
	NodeA.SetBaseCode("{0} + {1}");

	NodeA.GetInputPin(0)->LinkWith(NodeB.GetOutputPin(0));
	NodeA.GetInputPin(1)->LinkWith(NodeC.GetOutputPin(0));

	MTShaderCodeGenerator CodeGenerator;
	CodeGenerator.GenerateCode(&NodeA, TestRDIUtils::GetDX12ShaderFormat());

	MTString ExpectCode;

	//Metal
	{
#ifdef __APPLE__
		ExpectCode += "texture2d<float, access::sample> " + MTShaderCodeKeyword::TextureName() + "0[[texture(0)]];\n\t";
		ExpectCode += "sampler " + MTShaderCodeKeyword::SamplerName() + "0[[sampler(0)]];\n\t";
		ExpectCode += "texture2d<float, access::sample> " + MTShaderCodeKeyword::TextureName() + "1[[texture(1)]];\n\t";
		ExpectCode += "sampler " + MTShaderCodeKeyword::SamplerName() + "1[[sampler(1)]];\n\t";

		EXPECT_EQ(ExpectCode, CodeGenerator.GetTextureDeclarationCode(MTShaderPlatform::METAL));
#endif
	}
}

TEST(ShaderGraphMaterialNodeTest, RegenerateShaderCode)
{
	MTShaderGraphMaterialNode MaterialNode;

	MTShaderCodeGenerator CodeGenerator;
	CodeGenerator.GenerateCode(&MaterialNode, TestRDIUtils::GetDX12ShaderFormat());

	EXPECT_EQ(MTString(""), MaterialNode.GetShaderCode(TestRDIUtils::GetDX12ShaderFormat()));



	MTShaderGraphNode Node[9];
	for (MTInt32 i = 0; i < 9; ++i)
	{
		Node[i].InitOutput(1);
		Node[i].GetOutputPin(0)->LinkWith(MaterialNode.GetInputPin(i));
	}

	CodeGenerator.GenerateCode(&MaterialNode, TestRDIUtils::GetDX12ShaderFormat());

	MTString ExpectCode;
	ExpectCode += "Param.BaseColor = Node0.xyz;\n\t";
	ExpectCode += "Param.Metallic = Node1.x;\n\t";
	ExpectCode += "Param.Specular = Node2.x;\n\t";
	ExpectCode += "Param.Roughness = Node3.x;\n\t";
	ExpectCode += "Param.EmissiveColor = Node4.xyz;\n\t";
	ExpectCode += "Param.Opacity = Node5.x;\n\t";
	ExpectCode += "Param.OpacityMask = Node6.x;\n\t";
	ExpectCode += "Param.Normal = GetPixelShaderParamNormal(Node7.xyz, PixelShaderInput);\n\t";
	ExpectCode += "Param.WorldPositionOffset = Node8.xyz;";

	EXPECT_EQ(ExpectCode, MaterialNode.GetShaderCode(TestRDIUtils::GetDX12ShaderFormat()));
}
