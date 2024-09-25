#pragma once

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <ModelIO/ModelIO.h>

#include "MeshLoader.h"

class MetalMesh
{
public:
    id<MTLBuffer> vertexBuffer;
    MTLVertexDescriptor* vertexDescriptor;
    MTLPrimitiveType primitiveType;
    id<MTLBuffer> indexBuffer;
    int indexCount;
    MTLIndexType indexType;
    
    struct Submesh
    {
        id<MTLBuffer> indexBuffer;
        int indexCount;
    };
    MTArray<Submesh> Submeshes;
    
    struct Material
    {
        id<MTLTexture> DiffuseTexture;
        id<MTLTexture> NormalTexture;
    };
    MTArray<Material> Materials;
    
public:
    void Init(float cubeSize, id<MTLDevice> device)
    {
        MTKMeshBufferAllocator* allocator = [[MTKMeshBufferAllocator alloc] initWithDevice:device];
        
        MDLMesh* mdlMesh = [[MDLMesh alloc] initBoxWithExtent: vector_float3{cubeSize, cubeSize, cubeSize} segments: vector_uint3{10, 10, 10} inwardNormals: false geometryType:MDLGeometryTypeTriangles allocator: allocator];
        
        MTKMesh* mtkMesh = [[MTKMesh alloc] initWithMesh:mdlMesh device: device error:nullptr];
        MTKMeshBuffer* mtkVertexBuffer = mtkMesh.vertexBuffers[0];
        MTKSubmesh* submesh = mtkMesh.submeshes[0];
        MTKMeshBuffer* mtkIndexBuffer = submesh.indexBuffer;
        
        vertexBuffer = mtkVertexBuffer.buffer;
        vertexBuffer.label = @"Mesh Vertices";
        
        vertexDescriptor = MTKMetalVertexDescriptorFromModelIO(mdlMesh.vertexDescriptor);
        primitiveType = submesh.primitiveType;
        indexBuffer = mtkIndexBuffer.buffer;
        indexBuffer.label = @"Mesh Indices";
        
        indexCount = (int)submesh.indexCount;
        indexType = submesh.indexType;
        
        Submesh sm;
        sm.indexBuffer = mtkIndexBuffer.buffer;
        sm.indexCount = (int)submesh.indexCount;
        Submeshes.push_back(sm);
    }
    
    void InitFromObj(const MTString& FilePath, const MTString& MaterialDirectory, id<MTLDevice> device)
    {
        IMeshLoaderModule* MeshLoader = dynamic_cast<IMeshLoaderModule*>(ModuleManager()->LoadModule("MeshLoader"));
        if (MeshLoader)
        {
            MTObjMesh ObjMesh;
            if (MeshLoader->LoadObj(FilePath, MaterialDirectory, ObjMesh))
            {
                vertexBuffer = [device newBufferWithBytes:ObjMesh.VertexBuffer.data() length:(ObjMesh.VertexBuffer.size() * sizeof(MTObjMesh::Vertex)) options:0];
                
                for (const MTObjMesh::Submesh& ObjSubmesh : ObjMesh.Submeshes)
                {
                    MetalMesh::Submesh MetalSubmesh;
                    MetalSubmesh.indexBuffer = [device newBufferWithBytes:ObjSubmesh.IndexBuffer.data() length:(ObjSubmesh.IndexBuffer.size() * sizeof(MTUInt32)) options:0];
                    MetalSubmesh.indexCount = (int)ObjSubmesh.IndexBuffer.size();
                    
                    Submeshes.push_back(MetalSubmesh);
                }

                primitiveType = MTLPrimitiveTypeTriangle;
                indexType = MTLIndexTypeUInt32;
                
                for (const MTObjMesh::Material& ObjMaterial : ObjMesh.Materials)
                {
                    MetalMesh::Material MetalMaterial;
                    
                    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:device];
                    MTString TexturePath = ObjMesh.MaterialDirectory + ObjMaterial.DiffuseTexture;
                    NSURL* FilePath = [NSURL fileURLWithPath:[NSString stringWithUTF8String:TexturePath.c_str()]];
                    MetalMaterial.DiffuseTexture = [textureLoader newTextureWithContentsOfURL:FilePath options:nullptr error:nullptr];
                    
                    TexturePath = ObjMesh.MaterialDirectory + ObjMaterial.NormalTexture;
                    FilePath = [NSURL fileURLWithPath:[NSString stringWithUTF8String:TexturePath.c_str()]];
                    MetalMaterial.NormalTexture = [textureLoader newTextureWithContentsOfURL:FilePath options:nullptr error:nullptr];

                    Materials.push_back(MetalMaterial);
                }
            }
        }
    }
    
    MTInt32 GetSubmeshCount()
    {
        return Array::Size(Submeshes);
    }
};
