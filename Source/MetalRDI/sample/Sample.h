#pragma once

#import "simd/matrix_types.h"

@protocol MTLDevice;
class MetalMesh;
@protocol MTLRenderPipelineState;
@protocol MTLRenderCommandEncoder;
@protocol MTLDepthStencilState;
@protocol MTLSamplerState;
@protocol MTLTexture;


class MTMetalSample
{
public:
    ~MTMetalSample();
    
    void Init(id<MTLDevice> MetalDevice);
    
    void Render(id<MTLRenderCommandEncoder> renderEncoder);
    
private:
    id<MTLRenderPipelineState> buildRenderPipelineWithDevice();
    
    id<MTLDevice> m_MetalDevice;
    MetalMesh* m_Mesh = nullptr;
    id<MTLRenderPipelineState> m_RenderPipelineState;
    id<MTLDepthStencilState> m_DepthStencilState;
    id<MTLSamplerState> m_Sampler;
    id<MTLTexture> m_DiffuseTexture;
    id<MTLTexture> m_NormalTexture;
    
    struct ShaderConstants
    {
        //matrix_float4x4
        MTMatrix4x4 modelViewProjectionMatrix;
        //matrix_float3x3
        MTMatrix3x3 normalMatrix;
    };
    ShaderConstants m_Constants;
};
