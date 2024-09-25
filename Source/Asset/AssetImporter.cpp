#include "AssetImporter.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../ThirdParty/tinyobjloader/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct ObjVertex
{
	MTVector3 Position = MTVector3(0.0f, 0.0f, 0.0f);
	MTVector3 Normal = MTVector3(0.0f, 0.0f, 0.0f);
	MTVector3 Tangent = MTVector3(0.0f, 0.0f, 0.0f);
	MTVector3 Binormal = MTVector3(0.0f, 0.0f, 0.0f);
	MTVector2 UV = MTVector2(0.0f, 0.0f);
};

struct ObjVertexDataIndex
{
	MTInt32 PositionIndex;
	MTInt32 NormalIndex;
	MTInt32 UVIndex;

	bool operator<(const ObjVertexDataIndex& Other) const
	{
		if (PositionIndex < Other.PositionIndex)    return true;
		if (PositionIndex > Other.PositionIndex)    return false;
		if (NormalIndex < Other.NormalIndex)    return true;
		if (NormalIndex > Other.NormalIndex)    return false;
		if (UVIndex < Other.UVIndex)    return true;
		if (UVIndex > Other.UVIndex)    return false;

		return false;
	}
};

void MTTextureImportData::Clear()
{
	if (Pixels)
	{
		stbi_image_free(Pixels);
	}
}

bool MTAssetImporter::LoadObj(const MTString& FilePath, const MTString& MaterialDirectory, MTObjMesh& outMesh)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, FilePath.c_str(), MaterialDirectory.c_str());

	if (!err.empty()) // `err` may contain warning message.
	{
		std::cerr << err << std::endl;
	}

	if (!ret)
	{
		return false;
	}

	MTInt32 SubmeshCount = MTMath::Max(1, (MTInt32)materials.size());
	outMesh.Submeshes.Reserve(SubmeshCount);
	for (MTInt32 i = 0; i < SubmeshCount; ++i)
	{
		outMesh.Submeshes.Add(MTObjMesh::Submesh());
	}

	MTInt32 iSubmesh = 0;

	MTArray<ObjVertex> Vertices;
	MTMap<ObjVertexDataIndex, MTInt32> VertexDataIndexMap;

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			iSubmesh = MTMath::Max(0, shapes[s].mesh.material_ids[f]);

			int fv = shapes[s].mesh.num_face_vertices[f];

			MTInt32 FaceVertexIndex[3];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				MTInt32 VertexIndex;

				ObjVertexDataIndex VertexDataIndex;
				VertexDataIndex.PositionIndex = idx.vertex_index;
				VertexDataIndex.NormalIndex = idx.normal_index;
				VertexDataIndex.UVIndex = idx.texcoord_index;

				MTInt32* iVertexDataIndex = VertexDataIndexMap.Get(VertexDataIndex);
				if (iVertexDataIndex)
				{
					VertexIndex = *iVertexDataIndex;
				}
				else
				{
					VertexIndex = Vertices.Num();
					VertexDataIndexMap[VertexDataIndex] = VertexIndex;

					ObjVertex Vertex;
					if (idx.vertex_index >= 0)
					{
						Vertex.Position[0] = attrib.vertices[3 * idx.vertex_index + 0];
						Vertex.Position[1] = attrib.vertices[3 * idx.vertex_index + 1];
						Vertex.Position[2] = attrib.vertices[3 * idx.vertex_index + 2];
					}

					if (idx.normal_index >= 0)
					{
						Vertex.Normal[0] = attrib.normals[3 * idx.normal_index + 0];
						Vertex.Normal[1] = attrib.normals[3 * idx.normal_index + 1];
						Vertex.Normal[2] = attrib.normals[3 * idx.normal_index + 2];
					}

					if (idx.texcoord_index >= 0)
					{
						Vertex.UV[0] = attrib.texcoords[2 * idx.texcoord_index + 0];
						Vertex.UV[1] = attrib.texcoords[2 * idx.texcoord_index + 1];
					}

					Vertices.Add(Vertex);
				}

				outMesh.Submeshes[iSubmesh].IndexBuffer.Add(VertexIndex);

				if (v < 3)
				{
					FaceVertexIndex[v] = VertexIndex;
				}
			}

			ObjVertex& V0 = Vertices[FaceVertexIndex[0]];
			ObjVertex& V1 = Vertices[FaceVertexIndex[1]];
			ObjVertex& V2 = Vertices[FaceVertexIndex[2]];

			MTVector3 Tangent, Binormal;
			MTMath::ComputeVertexTangent(V0.Position, V1.Position, V2.Position, V0.UV, V1.UV, V2.UV, Tangent, Binormal);
			V0.Tangent += Tangent;
			V1.Tangent += Tangent;
			V2.Tangent += Tangent;
			V0.Binormal += Binormal;
			V1.Binormal += Binormal;
			V2.Binormal += Binormal;

			index_offset += fv;
		}
	}

	for (ObjVertex& Vertex : Vertices)
	{
		Vertex.Tangent = MTMath::OrthogonalizeVertexTangent(Vertex.Normal, Vertex.Tangent, Vertex.Binormal);
		Vertex.Binormal.Normalize();

		MTObjMesh::Vertex BufferVertex;
		BufferVertex.Position[0] = Vertex.Position[0];
		BufferVertex.Position[1] = Vertex.Position[1];
		BufferVertex.Position[2] = Vertex.Position[2];
		BufferVertex.Normal[0] = Vertex.Normal[0];
		BufferVertex.Normal[1] = Vertex.Normal[1];
		BufferVertex.Normal[2] = Vertex.Normal[2];
		BufferVertex.Tangent[0] = Vertex.Tangent[0];
		BufferVertex.Tangent[1] = Vertex.Tangent[1];
		BufferVertex.Tangent[2] = Vertex.Tangent[2];
		BufferVertex.UV[0] = Vertex.UV[0];
		BufferVertex.UV[1] = Vertex.UV[1];

		outMesh.VertexBuffer.Add(BufferVertex);
	}

	for (tinyobj::material_t material : materials)
	{
		MTObjMesh::Material ObjMaterial;
		ObjMaterial.DiffuseTexture = material.diffuse_texname;
		ObjMaterial.NormalTexture = material.bump_texname;

		outMesh.Materials.Add(ObjMaterial);
	}

	outMesh.MaterialDirectory = MaterialDirectory;

	if (outMesh.Submeshes.Num() != outMesh.Materials.Num())
	{
		std::cerr << "MTAssetImporter::LoadObj() - Submesh & Material count is not same." << std::endl;
	}

	return true;
}

MTTextureImportData MTAssetImporter::LoadTexture(const MTString& FilePath)
{
	MTTextureImportData ImportData;
	ImportData.Pixels = stbi_load(FilePath.c_str(), &ImportData.Width, &ImportData.Height, &ImportData.Channel, STBI_rgb_alpha);

	return ImportData;
}
