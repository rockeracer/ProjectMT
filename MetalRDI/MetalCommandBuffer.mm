#include "MetalAPI.pch"
#include "MetalCommandBuffer.h"
#include "MetalResource.h"

MTMetalCommandBuffer::MTMetalCommandBuffer(id<MTLCommandQueue> CommandQueue)
: m_CommandQueue(CommandQueue)
{
    
}

void MTMetalCommandBuffer::BeginCommand(MTRDIWindowView* View, bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth)
{
    MTMetalWindowView* MetalView = dynamic_cast<MTMetalWindowView*>(View);
    if (MetalView)
    {
        CreateCommandEncoder([(id<CAMetalDrawable>)MetalView->CurrentMetalDrawable texture], MetalView->DepthBuffer, bClearColor, ClearColor, bClearDepth, ClearDepth);
    }
}

void MTMetalCommandBuffer::BeginCommand(MTRDITexture* ColorBuffer, MTRDITexture* DepthBuffer,
                     bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth)
{
    MTMetalTexture* MetalColorBuffer = dynamic_cast<MTMetalTexture*>(ColorBuffer);
    MTMetalTexture* MetalDepthBuffer = dynamic_cast<MTMetalTexture*>(DepthBuffer);
    
    if (MetalColorBuffer && MetalDepthBuffer)
    {
        CreateCommandEncoder(MetalColorBuffer->Texture, MetalDepthBuffer->Texture, bClearColor, ClearColor, bClearDepth, ClearDepth);
    }
}

void MTMetalCommandBuffer::CreateCommandEncoder(id<MTLTexture> ColorBuffer, id<MTLTexture> DepthBuffer,
                                        bool bClearColor, MTColor ClearColor, bool bClearDepth, float ClearDepth)
{
    m_CommandBuffer = [m_CommandQueue commandBuffer];
    
    MTLRenderPassDescriptor* RenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    RenderPassDescriptor.colorAttachments[0].texture = ColorBuffer;
    RenderPassDescriptor.colorAttachments[0].loadAction = bClearColor ? MTLLoadActionClear : MTLLoadActionLoad;
    RenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    RenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
    RenderPassDescriptor.depthAttachment.texture = DepthBuffer;
    RenderPassDescriptor.depthAttachment.loadAction = bClearDepth ? MTLLoadActionClear : MTLLoadActionLoad;;
    RenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    RenderPassDescriptor.depthAttachment.clearDepth = ClearDepth;
    
    m_CommandEncoder = [m_CommandBuffer renderCommandEncoderWithDescriptor:RenderPassDescriptor];
}

void MTMetalCommandBuffer::EndCommand()
{
    if (m_CommandEncoder)
    {
        [m_CommandEncoder endEncoding];
        [m_CommandBuffer commit];
    }
}

void MTMetalCommandBuffer::SetShader(MTRDIShader* Shader)
{
    MTMetalShader* MetalShader = dynamic_cast<MTMetalShader*>(Shader);
    if (MetalShader)
    {
        [m_CommandEncoder setRenderPipelineState:MetalShader->RenderPipelineState];
    }
}

void MTMetalCommandBuffer::SetMaterial(MTRDIMaterial* Material)
{
    if (Material)
    {
        for (MTInt32 i = 0; i < Material->Textures.size(); ++i)
        {
            MTMetalTexture* MetalTexture = dynamic_cast<MTMetalTexture*>(Material->Textures[i]);
            if (MetalTexture)
            {
                [m_CommandEncoder setFragmentTexture:MetalTexture->Texture atIndex:i];
                [m_CommandEncoder setFragmentSamplerState:MetalTexture->Sampler atIndex:i];
            }
        }
    }
}

void MTMetalCommandBuffer::SetVertexBuffer(MTRDIBuffer* VertexBuffer, MTInt32 Index, MTInt32 Offset)
{
    MTMetalBuffer* MetalVertexBuffer = dynamic_cast<MTMetalBuffer*>(VertexBuffer);
    if (MetalVertexBuffer)
    {
        [m_CommandEncoder setVertexBuffer:MetalVertexBuffer->Buffer offset:Offset atIndex:Index];
    }
}

void MTMetalCommandBuffer::SetConstantBuffer(void* ConstantData, MTInt32 Size, MTInt32 Index)
{
    if (Data)
    {
        [m_CommandEncoder setVertexBytes:ConstantData length:Size atIndex:Index];
    }
}

void MTMetalCommandBuffer::SetDepthStencilState(MTRDIDepthStencilState* DepthStencilState)
{
    MTMetalDepthStencilState* MetalDepthStencilState = dynamic_cast<MTMetalDepthStencilState*>(DepthStencilState);
    if (MetalDepthStencilState)
    {
        [m_CommandEncoder setDepthStencilState:MetalDepthStencilState->DepthStencilState];
    }
}

void MTMetalCommandBuffer::SetTriangleFacingWinding(MTTriangleFacingWinding Winding)
{
    [m_CommandEncoder setFrontFacingWinding:MTMetalTypeConverter::Winding(Winding)];
}

void MTMetalCommandBuffer::SetCullMode(MTCullMode CullMode)
{
    [m_CommandEncoder setCullMode:MTMetalTypeConverter::CullMode(CullMode)];
}
void MTMetalCommandBuffer::DrawMesh(MTRDIMesh* Mesh, MTInt32 SubmeshIndex)
{
    if (Mesh)
    {
        MTLPrimitiveType PrimitiveType = MTMetalTypeConverter::PrimitiveType(Mesh->PrimitiveType);
        MTInt32 IndexCount = Mesh->Submeshes[SubmeshIndex].IndexCount;
        MTLIndexType IndexType = MTMetalTypeConverter::IndexType(Mesh->IndexType);
        id<MTLBuffer> IndexBuffer;
        MTMetalBuffer* MetalIndexBuffer = dynamic_cast<MTMetalBuffer*>(Mesh->Submeshes[SubmeshIndex].IndexBuffer);
        if (MetalIndexBuffer)
        {
            IndexBuffer = MetalIndexBuffer->Buffer;
        }
        [m_CommandEncoder drawIndexedPrimitives:PrimitiveType indexCount:IndexCount indexType:IndexType indexBuffer:IndexBuffer indexBufferOffset:0];
    }
}

void MTMetalCommandBuffer::SetPixelTexture(MTRDITexture* Texture, MTInt32 Index)
{
    MTMetalTexture* MetalTexture = dynamic_cast<MTMetalTexture*>(Texture);
    if (MetalTexture)
    {
        [m_CommandEncoder setFragmentTexture:MetalTexture->Texture atIndex:Index];
        [m_CommandEncoder setFragmentSamplerState:MetalTexture->Sampler atIndex:Index];
    }
}
