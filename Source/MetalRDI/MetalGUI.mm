#include "MetalAPI.pch"
#include "MetalGUI.h"
#include "MetalResource.h"

#include "GUIRender.h"
#include "RenderResource.h"

#include "imgui.h"


#include "Sample.h"
MTMetalSample g_Sample;


MTMetalGUI::MTMetalGUI(id<MTLDevice> MetalDevice, id<MTLCommandQueue> MetalCommandQueue)
: m_MetalDevice(MetalDevice), m_MetalCommandQueue(MetalCommandQueue)
{
    g_Sample.Init(m_MetalDevice);
}

void* MTMetalGUI::Init(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc)
{
    MTLTextureDescriptor *fontTextureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
                                                                                                     width:FontTextureDesc.Width
                                                                                                    height:FontTextureDesc.Height
                                                                                                 mipmapped:NO];
    m_MetalFontTexture = [m_MetalDevice newTextureWithDescriptor:fontTextureDescriptor];
    MTLRegion region = MTLRegionMake2D(0, 0, FontTextureDesc.Width, FontTextureDesc.Height);
    [m_MetalFontTexture replaceRegion:region mipmapLevel:0 withBytes:FontTextureDesc.Pixels bytesPerRow:FontTextureDesc.Width * sizeof(uint8_t)];
    
    
    MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
    
    m_MetalSamplerState = [m_MetalDevice newSamplerStateWithDescriptor:samplerDescriptor];
    
    NSString *shaders = @"#include <metal_stdlib>\n\
    using namespace metal;                                                                  \n\
    \n\
    struct vertex_t {                                                                       \n\
    float2 position [[attribute(0)]];                                                   \n\
    float2 tex_coords [[attribute(1)]];                                                 \n\
    uchar4 color [[attribute(2)]];                                                      \n\
    };                                                                                      \n\
    \n\
    struct frag_data_t {                                                                    \n\
    float4 position [[position]];                                                       \n\
    float4 color;                                                                       \n\
    float2 tex_coords;                                                                  \n\
    };                                                                                      \n\
    \n\
    vertex frag_data_t vertex_function(vertex_t vertex_in [[stage_in]],                     \n\
    constant float4x4 &proj_matrix [[buffer(1)]])        \n\
    {                                                                                       \n\
    float2 position = vertex_in.position;                                               \n\
    \n\
    frag_data_t out;                                                                    \n\
    out.position = proj_matrix * float4(position.xy, 0, 1);                             \n\
    out.color = float4(vertex_in.color) * (1 / 255.0);                                  \n\
    out.tex_coords = vertex_in.tex_coords;                                              \n\
    return out;                                                                         \n\
    }                                                                                       \n\
    \n\
    fragment float4 fragment_function(frag_data_t frag_in [[stage_in]],                     \n\
    texture2d<float, access::sample> tex [[texture(0)]],  \n\
    sampler tex_sampler [[sampler(0)]])                   \n\
    {                                                                                       \n\
    return frag_in.color * float4(tex.sample(tex_sampler, frag_in.tex_coords).r);       \n\
    }";
    
    NSError *error = nil;
    id<MTLLibrary> library = [m_MetalDevice newLibraryWithSource:shaders options:nil error:&error];
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_function"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_function"];
    
    if (!library || !vertexFunction || !fragmentFunction)
    {
        NSLog(@"Could not create library from shader source and retrieve functions");
        return nullptr;
    }
    
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    vertexDescriptor.attributes[0].offset = VertexDesc.PosOffset;
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].bufferIndex = 0;
    vertexDescriptor.attributes[1].offset = VertexDesc.UVOffset;
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[1].bufferIndex = 0;
    vertexDescriptor.attributes[2].offset = VertexDesc.ColorOffset;
    vertexDescriptor.attributes[2].format = MTLVertexFormatUChar4;
    vertexDescriptor.attributes[2].bufferIndex = 0;
    vertexDescriptor.layouts[0].stride = VertexDesc.VertexSize;
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
#undef OFFSETOF
    
    MTLRenderPipelineDescriptor *renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    renderPipelineDescriptor.vertexFunction = vertexFunction;
    renderPipelineDescriptor.fragmentFunction = fragmentFunction;
    renderPipelineDescriptor.vertexDescriptor = vertexDescriptor;
    renderPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    renderPipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
    renderPipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    renderPipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    renderPipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    renderPipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    
    m_MetalRenderPipelineState = [m_MetalDevice newRenderPipelineStateWithDescriptor:renderPipelineDescriptor error:&error];
    
    if (!m_MetalRenderPipelineState)
    {
        NSLog(@"Error when creating pipeline state: %@", error);
        return nullptr;
    }
    
    m_MtlBufferPool = [NSMutableArray array];
    
    return (void *)(intptr_t)m_MetalFontTexture;
}

void MTMetalGUI::Destroy()
{
    m_MetalFontTexture = nullptr;
    m_MetalSamplerState = nullptr;
    m_MetalRenderPipelineState = nullptr;
    m_MtlBufferPool = nullptr;
}

void MTMetalGUI::RenderGUI(MTRenderWindowView* View, const MTGUIViewportDesc& Viewport, ImDrawData* DrawData)
{
    MTMetalWindowView* MetalView = dynamic_cast<MTMetalWindowView*>(View);
    if (MetalView == nullptr)
    {
        return;
    }
    
    id<MTLCommandBuffer> commandBuffer = [m_MetalCommandQueue commandBuffer];
    
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = [(id<CAMetalDrawable>)MetalView->CurrentMetalDrawable texture];
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1);
    
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    
    [commandEncoder setRenderPipelineState:m_MetalRenderPipelineState];
    
    MTLViewport viewport = {
        .originX = 0, .originY = 0, .width = (double)Viewport.ViewportWidth, .height = (double)Viewport.ViewportHeight, .znear = 0, .zfar = 1
    };
    [commandEncoder setViewport:viewport];
    
    float left = 0, right = Viewport.DisplayWidth, top = 0, bottom = Viewport.DisplayHeight;
    float near = 0;
    float far = 1;
    float sx = 2 / (right - left);
    float sy = 2 / (top - bottom);
    float sz = 1 / (far - near);
    float tx = (right + left) / (left - right);
    float ty = (top + bottom) / (bottom - top);
    float tz = near / (far - near);
    float orthoMatrix[] = {
        sx,  0,  0, 0,
        0, sy,  0, 0,
        0,  0, sz, 0,
        tx, ty, tz, 1
    };
    
    [commandEncoder setVertexBytes:orthoMatrix length:sizeof(float) * 16 atIndex:1];
    
    // Render command lists
    for (int n = 0; n < DrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = DrawData->CmdLists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
        const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        
        NSUInteger vertexBufferSize = sizeof(ImDrawVert) * cmd_list->VtxBuffer.size();
        id<MTLBuffer> vertexBuffer = DequeueReusableBuffer(vertexBufferSize);
        memcpy([vertexBuffer contents], vtx_buffer, vertexBufferSize);
        
        NSUInteger indexBufferSize = sizeof(ImDrawIdx) * cmd_list->IdxBuffer.size();
        id<MTLBuffer> indexBuffer = DequeueReusableBuffer(indexBufferSize);
        memcpy([indexBuffer contents], idx_buffer, indexBufferSize);
        
        [commandEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        
        int idx_buffer_offset = 0;
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                MTLScissorRect scissorRect = {
                    .x = (NSUInteger)pcmd->ClipRect.x,
                    .y = (NSUInteger)(pcmd->ClipRect.y),
                    .width = (NSUInteger)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                    .height = (NSUInteger)(pcmd->ClipRect.w - pcmd->ClipRect.y)
                };
                
                if (scissorRect.x + scissorRect.width <= Viewport.ViewportWidth && scissorRect.y + scissorRect.height <= Viewport.ViewportHeight)
                {
                    [commandEncoder setScissorRect:scissorRect];
                }
                
                [commandEncoder setFragmentTexture:(__bridge id<MTLTexture>)pcmd->TextureId atIndex:0];
                
                [commandEncoder setFragmentSamplerState:m_MetalSamplerState atIndex:0];
                
                [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                           indexCount:(MTInt32)pcmd->ElemCount
                                            indexType:sizeof(ImDrawIdx) == 2 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32
                                          indexBuffer:indexBuffer
                                    indexBufferOffset:sizeof(ImDrawIdx) * idx_buffer_offset];
            }
            
            idx_buffer_offset += pcmd->ElemCount;
        }
        
        //dispatch_queue_t queue = dispatch_get_current_queue();
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
            //dispatch_async(queue, ^{
                EnqueueReusableBuffer(vertexBuffer);
                EnqueueReusableBuffer(indexBuffer);
            //});
        }];
    }
    
    [commandEncoder endEncoding];
    
    [commandBuffer commit];
}

id<MTLBuffer> MTMetalGUI::DequeueReusableBuffer(NSUInteger size)
{
    for (MTInt32 i = 0; i < [m_MtlBufferPool count]; ++i)
    {
        id<MTLBuffer> candidate = m_MtlBufferPool[i];
        if ([candidate length] >= size)
        {
            [m_MtlBufferPool removeObjectAtIndex:i];
            return candidate;
        }
    }
    
    return [m_MetalDevice newBufferWithLength:size options:MTLResourceCPUCacheModeDefaultCache];
}

void MTMetalGUI::EnqueueReusableBuffer(id<MTLBuffer> buffer)
{
    [m_MtlBufferPool insertObject:buffer atIndex:0];
}

void MTMetalGUI::BeginRender(MTRenderWindowView* View)
{
    MTMetalWindowView* MetalView = dynamic_cast<MTMetalWindowView*>(View);
    if (MetalView == nullptr)
    {
        return;
    }
    
    id<MTLCommandQueue> commandQueue = m_MetalCommandQueue;
    id<CAMetalDrawable> currentDrawable = MetalView->CurrentMetalDrawable;
    id<MTLCommandBuffer> clearBuffer = [commandQueue commandBuffer];
    ImVec4 clear_color = ImColor(114, 144, 154);
    MTLRenderPassDescriptor *clearPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    clearPassDescriptor.colorAttachments[0].texture = [currentDrawable texture];
    clearPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    clearPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    clearPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    clearPassDescriptor.depthAttachment.texture = MetalView->DepthBuffer;
    clearPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    clearPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    clearPassDescriptor.depthAttachment.clearDepth = 1.0f;
    
    id<MTLRenderCommandEncoder> clearEncoder = [clearBuffer renderCommandEncoderWithDescriptor:clearPassDescriptor];
    // You could draw your scene or anything else in here; GUI is drawn in a separate pass below
    //g_Sample.Render(clearEncoder);
    [clearEncoder endEncoding];
    [clearBuffer commit];

}

void MTMetalGUI::EndRender(MTRenderWindowView* View)
{
    MTMetalWindowView* MetalView = dynamic_cast<MTMetalWindowView*>(View);
    if (MetalView)
    {
        m_PresentCommandBuffer = [m_MetalCommandQueue commandBuffer];
        [m_PresentCommandBuffer presentDrawable:MetalView->CurrentMetalDrawable];
        [m_PresentCommandBuffer commit];
    }
}

void MTMetalGUI::WaitForRenderCompleted()
{
    if (m_PresentCommandBuffer)
    {
        [m_PresentCommandBuffer waitUntilCompleted];
    }
}
