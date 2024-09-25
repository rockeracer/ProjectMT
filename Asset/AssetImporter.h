#pragma once

#include "Asset.h"

struct MTObjMesh
{
	struct Vertex
	{
		float Position[3];
		float Normal[3];
		float Tangent[3];
		float UV[2];
	};

	struct Submesh
	{
		MTArray<MTUInt32> IndexBuffer;
	};

	struct Material
	{
		MTString DiffuseTexture;
		MTString NormalTexture;
	};

	MTArray<Vertex> VertexBuffer;

	MTArray<Submesh> Submeshes;
	MTArray<Material> Materials;

	MTString MaterialDirectory;
};

struct ASSET_API MTTextureImportData
{
	MTInt32 Width = 0;
	MTInt32 Height = 0;
	MTInt32 Channel = 0;
	MTUChar* Pixels = nullptr;

	MTInt32 GetTextureSize() const
	{
		return Width * Height * Channel;
	}

	void Clear();
};

class ASSET_API MTAssetImporter
{
public:
	static bool LoadObj(const MTString& FilePath, const MTString& MaterialDirectory, MTObjMesh& outMesh);

	static MTTextureImportData LoadTexture(const MTString& FilePath);
};
