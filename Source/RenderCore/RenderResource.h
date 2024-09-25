#pragma once

#include "RDI.h"
#include "Shader.h"

class MTRenderResource
{
public:
	virtual ~MTRenderResource()
	{

	}
};

class MTRenderTexture : public MTRenderResource
{
public:
	virtual ~MTRenderTexture()
	{
		if (m_RDITexture)
		{
			delete m_RDITexture;
		}
	}

	void Init(MTInt32 Width, MTInt32 Height, MTUChar* Pixels, MTInt32 Size)
	{
		IRDI* RDI = RDI::GetRDI();
		if (RDI)
		{
			m_RDITexture = RDI->CreateTextureFromMemory(Width, Height, Pixels, Size);
		}
	}

	MTRDITexture* GetRDITexture() const
	{
		return m_RDITexture;
	}

private:
	MTRDITexture* m_RDITexture = nullptr;
};

class MTRenderMaterial : public MTRenderResource
{
public:
	virtual ~MTRenderMaterial()
	{
		if (m_RDIMaterial)
		{
			delete m_RDIMaterial;
		}
	}

	void Init(MTArray<MTRenderTexture*> RenderTextures)
	{
		IRDI* RDI = RDI::GetRDI();
		if (RDI)
		{
			IShaderModule* ShaderModule = Core::LoadModule<IShaderModule>("Shader");
			MTShaderCode ShaderCode = ShaderModule->GetMaterialShaderCode(RDI->GetShaderCodeFormat());

			MTArray<MTRDITexture*> RDITextures;
			for (MTRenderTexture* RenderTexture : RenderTextures)
			{
				RDITextures.Add(RenderTexture->GetRDITexture());
			}

			m_RDIMaterial = RDI->CreateMaterial(ShaderCode.VertexShaderCode, ShaderCode.PixelShaderCode, RDITextures);
		}
	}

private:
	MTRDIMaterial* m_RDIMaterial = nullptr;
};

class MTRenderStaticMesh : public MTRenderResource
{
public:
	virtual ~MTRenderStaticMesh()
	{
		ReleaseRDIResource();
	}
	void CreateVertexBuffer(char* VertexData, MTUInt32 VertexSize, MTUInt32 VertexCount)
	{
		MT_ASSERT(m_VertexBuffer == nullptr);

		IRDI* RDI = RDI::GetRDI();
		if (RDI)
		{
			m_VertexBuffer = RDI->CreateVertexBuffer(VertexData, VertexSize * VertexCount, VertexSize);
		}
	}

	void AddIndexBuffer(void* IndexData, MTUInt32 IndexSize, MTUInt32 IndexCount)
	{
		IRDI* RDI = RDI::GetRDI();
		if (RDI)
		{
			MTRDIBuffer* IndexBuffer = RDI->CreateIndexBuffer(IndexData, IndexCount * IndexSize, IndexSize);
			m_IndexBuffers.Add(IndexBuffer);
		}
	}

private:
	void ReleaseRDIResource()
	{
		if (m_VertexBuffer)
		{
			delete m_VertexBuffer;
			m_VertexBuffer = nullptr;
		}

		for (MTRDIBuffer* IndexBuffer : m_IndexBuffers)
		{
			if (IndexBuffer)
			{
				delete IndexBuffer;
			}
		}
		m_IndexBuffers.Clear();
	}

	MTRDIBuffer* m_VertexBuffer = nullptr;

	MTArray<MTRDIBuffer*> m_IndexBuffers;

	MTArray<MTRDIMaterial*> m_Materials;
};