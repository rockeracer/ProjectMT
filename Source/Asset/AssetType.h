#pragma once

#include "AssetImporter.h"

#include "RenderResource.h"

class MTTexture
{
public:
	~MTTexture()
	{
		if (m_RenderTexture)
		{
			delete m_RenderTexture;
		}
	}

	void InitFromFile(const MTString& FilePath)
	{
		MTTextureImportData TextureImport = MTAssetImporter::LoadTexture(FilePath);
		m_RenderTexture = new MTRenderTexture;
		m_RenderTexture->Init(TextureImport.Width, TextureImport.Height, TextureImport.Pixels, TextureImport.GetTextureSize());
		TextureImport.Clear();
	}

	MTRenderTexture* GetRenderTexture() const
	{
		return m_RenderTexture;
	}

private:
	MTRenderTexture* m_RenderTexture = nullptr;
};

class MTMaterial
{
public:
	~MTMaterial()
	{
		if (m_RenderMaterial)
		{
			delete m_RenderMaterial;
		}
	}

	void Init(MTArray<MTTexture*> Textures)
	{
		m_Textures = Textures;

		MTArray<MTRenderTexture*> RenderTextures;
		for (MTTexture* Texture : Textures)
		{
			RenderTextures.Add(Texture->GetRenderTexture());
		}

		m_RenderMaterial = new MTRenderMaterial;
		m_RenderMaterial->Init(RenderTextures);
	}

private:
	MTArray<MTTexture*> m_Textures;

	MTRenderMaterial* m_RenderMaterial = nullptr;
};

class MTStaticMesh
{
public:
	~MTStaticMesh()
	{
		if (m_RenderStaticMesh)
		{
			delete m_RenderStaticMesh;
		}

		DeallocateBuffer();

		// #TODO : Must release from SmartPointer
		{
			for (MTMaterial* Material : m_Materials)
			{
				delete Material;
			}

			for (MTTexture* Texture : m_Textures)
			{
				delete Texture;
			}
		}
	}

	void InitFromFile(const MTString& FilePath, const MTString& MaterialDirectory)
	{
		MTObjMesh ObjMesh;
		if (MTAssetImporter::LoadObj(FilePath, MaterialDirectory, ObjMesh))
		{
			m_RenderStaticMesh = new MTRenderStaticMesh;

			m_VertexCount = ObjMesh.VertexBuffer.Num();
			MTInt32 VertexBufferSize = m_VertexCount * sizeof(MTObjMesh::Vertex);
			m_VertexData = new char[VertexBufferSize];
			memcpy(m_VertexData, ObjMesh.VertexBuffer.Data(), VertexBufferSize);

			m_RenderStaticMesh->CreateVertexBuffer(m_VertexData, sizeof(MTObjMesh::Vertex), m_VertexCount);

			for (const MTObjMesh::Submesh& ObjSubmesh : ObjMesh.Submeshes)
			{
				MTStaticMesh::Submesh Submesh;
				Submesh.IndexCount = ObjSubmesh.IndexBuffer.Num();
				MTInt32 IndexBufferSize = Submesh.IndexCount * sizeof(MTUInt32);
				Submesh.IndexData = new MTUInt32[Submesh.IndexCount];
				memcpy(Submesh.IndexData, ObjSubmesh.IndexBuffer.Data(), IndexBufferSize);

				m_Submeshes.Add(Submesh);

				m_RenderStaticMesh->AddIndexBuffer(Submesh.IndexData, sizeof(MTUInt32), Submesh.IndexCount);
			}

			for (const MTObjMesh::Material& ObjMaterial : ObjMesh.Materials)
			{
				MTArray<MTTexture*> Textures(2);
				MTString TexturePath = ObjMesh.MaterialDirectory + ObjMaterial.DiffuseTexture;
				Textures[0] = new MTTexture;
				Textures[0]->InitFromFile(TexturePath);
				m_Textures.Add(Textures[0]);

				TexturePath = ObjMesh.MaterialDirectory + ObjMaterial.NormalTexture;
				Textures[1] = new MTTexture;
				Textures[1]->InitFromFile(TexturePath);
				m_Textures.Add(Textures[1]);

				MTMaterial* Material = new MTMaterial;
				Material->Init(Textures);

				m_Materials.Add(Material);
			}
		}
	}

private:
	void DeallocateBuffer()
	{
		if (m_VertexData)
		{
			delete[] m_VertexData;
			m_VertexData = nullptr;
		}

		for (MTStaticMesh::Submesh& Submesh : m_Submeshes)
		{
			if (Submesh.IndexData)
			{
				delete[] Submesh.IndexData;
			}
		}
		m_Submeshes.Clear();
	}

	MTInt32 m_VertexCount = 0;
	MTInt32 m_VertexSize = 0;
	char* m_VertexData = nullptr;

	MTInt32 m_IndexSize = sizeof(MTUInt32);

	struct Submesh
	{
		void* IndexData = nullptr;
		MTInt32 IndexCount = 0;
	};
	MTArray<Submesh> m_Submeshes;

	MTArray<MTTexture*> m_Textures;

	MTArray<MTMaterial*> m_Materials;

	MTRenderStaticMesh* m_RenderStaticMesh = nullptr;
};
