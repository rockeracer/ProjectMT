#include "Sample.h"
#include "Mesh.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "AAPLMathUtilities.h"

#include "MeshLoader.h"
#include "Shader.h"

MTMetalSample::~MTMetalSample()
{
    if (m_Mesh)
    {
        delete m_Mesh;
    }
}

void MTMetalSample::Init(id<MTLDevice> MetalDevice)
{
    m_MetalDevice = MetalDevice;
    m_Mesh = new MetalMesh;
    //m_Mesh->Init(1, m_MetalDevice);
    m_Mesh->InitFromObj("../Data/temple.obj", "../Data/", MetalDevice);
    
    m_RenderPipelineState = buildRenderPipelineWithDevice();
    
    MTLDepthStencilDescriptor* DepthStencilDesc = [MTLDepthStencilDescriptor new];
    DepthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
    DepthStencilDesc.depthWriteEnabled = YES;
    m_DepthStencilState = [m_MetalDevice newDepthStencilStateWithDescriptor:DepthStencilDesc];
    
    MTLSamplerDescriptor* samplerDescriptor = [MTLSamplerDescriptor new];
    samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
    samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
    m_Sampler = [m_MetalDevice newSamplerStateWithDescriptor:samplerDescriptor];

    
    
    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:m_MetalDevice];
    NSURL* FilePath = [NSURL fileURLWithPath:@"../Data/checkerboard.png"];
    m_DiffuseTexture = [textureLoader newTextureWithContentsOfURL:FilePath options:nullptr error:nullptr];
    FilePath = [NSURL fileURLWithPath:@"../Data/brick_normal.png"];
    m_NormalTexture = [textureLoader newTextureWithContentsOfURL:FilePath options:nullptr error:nullptr];
}

id<MTLRenderPipelineState> MTMetalSample::buildRenderPipelineWithDevice()
{
    id<MTLLibrary> Library = [m_MetalDevice newDefaultLibrary];
    
    IShaderModule* ShaderModule = dynamic_cast<IShaderModule*>(ModuleManager()->LoadModule("Shader"));
    MTString ShaderCode = ShaderModule->GetMaterialShaderCode();
    
    NSError* Error = nil;
    Library = [m_MetalDevice newLibraryWithSource:[NSString stringWithUTF8String:ShaderCode.c_str()] options:nil error:&Error];
    
    // Retrieve the functions that will comprise our pipeline
    id<MTLFunction> vertexFunction = [Library newFunctionWithName:@"vertex_transform"];//Library.makeFunction(name: "vertex_transform")
    id<MTLFunction> fragmentFunction = [Library newFunctionWithName:@"fragment_lit_textured"];
    
    // A render pipeline descriptor describes the configuration of our programmable pipeline
    MTLRenderPipelineDescriptor* pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineDescriptor.label = @"Render Pipeline";
    //pipelineDescriptor.sampleCount = view.sampleCount
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;//view.colorPixelFormat;
    pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;//view.depthStencilPixelFormat;

    return [m_MetalDevice newRenderPipelineStateWithDescriptor:pipelineDescriptor error:nullptr];
}

void MTMetalSample::Render(id<MTLRenderCommandEncoder> renderEncoder)
{
//    let commandBuffer = commandQueue.makeCommandBuffer()
//    
//    // Ask the view for a configured render pass descriptor. It will have a loadAction of
//    // MTLLoadActionClear and have the clear color of the drawable set to our desired clear color.
//    let renderPassDescriptor = view.currentRenderPassDescriptor
//    
//    if let renderPassDescriptor = renderPassDescriptor {
//        // Create a render encoder to clear the screen and draw our objects
//        let renderEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)
//        
//        renderEncoder.pushDebugGroup(@"Draw Cube")
    
    MTMatrix4x4 modelToWorldMatrix = MTMatrix4x4(1, 0, 0, 0,
                                                 0, 1, 0, 0,
                                                 0, 0, 1, 0,
                                                 0, 0, 0, 1);
    static float Angle = 0.f;
    //Angle += 0.003;
    modelToWorldMatrix = MTMatrix4x4::FromRotationMatrix(MTMatrix3x3::RotationY(Angle));
    //matrix4x4_rotation(float(0) * 0.5, vector_float3(0.7, 1, 0));
    
    // So that the figure doesn't get distorted when the window changes size or rotates,
    // we factor the current aspect ration into our projection matrix. We also select
    // sensible values for the vertical view angle and the distances to the near and far planes.
    //let viewSize = self.view.bounds.size
    float aspectRatio = (1024.f / 768.f);
    float verticalViewAngle = radians_from_degrees(65);
    float nearZ = 0.1;
    float farZ = 100000.0;
    
//    matrix_float4x4 projectionMatrix = matrix_perspective(verticalViewAngle, aspectRatio, nearZ, farZ);
//    matrix_float4x4 viewMatrix = matrix_look_at(0, 0, 2.5, 0, 0, 0, 0, 1, 0);
//    matrix_float4x4 mvMatrix = viewMatrix;//matrix_multiply(viewMatrix, modelToWorldMatrix);
    
    MTMatrix4x4 projectionMatrix = MTMatrix4x4::Perspective(verticalViewAngle, aspectRatio, nearZ, farZ, -1);
    MTMatrix4x4 viewMatrix = MTMatrix4x4::LookAt(MTVector3(0, 0, 0), MTVector3(0, 800, 1500), MTVector3(0, 1, 0));
    
    // The combined model-view-projection matrix moves our vertices from model space into clip space
    MTMatrix4x4 mvMatrix = viewMatrix * modelToWorldMatrix;
    m_Constants.modelViewProjectionMatrix =projectionMatrix * mvMatrix;
    //m_Constants.modelViewProjectionMatrix = matrix_multiply(projectionMatrix, mvMatrix);
    m_Constants.normalMatrix = MTMatrix3x3::Identity();//matrix_inverse_transpose(matrix_upper_left_3x3(mvMatrix))
    
    
    
    // Since we specified the vertices of our triangles in counter-clockwise
    // order, we need to switch from the default of clockwise winding.
    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [renderEncoder setDepthStencilState:m_DepthStencilState];
    [renderEncoder setRenderPipelineState:m_RenderPipelineState];
    [renderEncoder setVertexBuffer:m_Mesh->vertexBuffer offset:0 atIndex:0];
    [renderEncoder setVertexBytes:&m_Constants length:sizeof(ShaderConstants) atIndex:1];
    [renderEncoder setFragmentTexture:m_DiffuseTexture atIndex:0];
    [renderEncoder setFragmentTexture:m_NormalTexture atIndex:1];
    [renderEncoder setFragmentSamplerState:m_Sampler atIndex:0];
    
    for (MTInt32 i = 0 ; i< m_Mesh->GetSubmeshCount(); ++i)
    {
        if (Array::IsValidIndex(m_Mesh->Materials, i))
        {
            MetalMesh::Material Material = m_Mesh->Materials[i];
            if (Material.DiffuseTexture)
            {
                [renderEncoder setFragmentTexture:Material.DiffuseTexture atIndex:0];
            }
            if (Material.NormalTexture)
            {
                [renderEncoder setFragmentTexture:Material.NormalTexture atIndex:1];
            }
        }
        
        [renderEncoder drawIndexedPrimitives:m_Mesh->primitiveType indexCount:m_Mesh->Submeshes[i].indexCount indexType:m_Mesh->indexType indexBuffer:m_Mesh->Submeshes[i].indexBuffer indexBufferOffset:0];
    }
    
    //[renderEncoder endEncoding];
}
