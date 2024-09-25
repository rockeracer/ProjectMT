/*
    Copyright (C) 2016 Apple Inc. All Rights Reserved.
    See LICENSE.txt for this sample’s licensing information
    
    Abstract:
    Shader file with functions for rendering lit, textured geometry.
*/

#include <metal_stdlib>
using namespace metal;

struct Constants {
    float4x4 modelViewProjectionMatrix;
    float3x3 normalMatrix;
};

constant half3 ambientLightIntensity(0.1, 0.1, 0.1);
constant half3 diffuseLightIntensity(0.9, 0.9, 0.9);
constant half3 lightDirection(-0.577, -0.577, -0.577);

struct VertexIn {
    packed_float3 position;
    packed_float3 normal;
    packed_float3 tangent;
    packed_float2 texCoords;
};

struct VertexOut {
    float4 position [[position]];
    float3 normal;
    float3 tangent;
    float2 texCoords;
    float NdotT;
};

vertex VertexOut vertex_transform(device VertexIn *vertices [[buffer(0)]],
                                  constant Constants &uniforms [[buffer(1)]],
                                  uint vertexId [[vertex_id]])
{
    float3 modelPosition = vertices[vertexId].position;
    float3 modelNormal = vertices[vertexId].normal;
    float3 modelTangent = vertices[vertexId].tangent;
    
    VertexOut out;
    // Multiplying the model position by the model-view-projection matrix moves us into clip space
    out.position = uniforms.modelViewProjectionMatrix * float4(modelPosition, 1);
    // Copy the vertex normal and texture coordinates
    out.normal = normalize(uniforms.normalMatrix * modelNormal);
    out.tangent = normalize(uniforms.normalMatrix * modelTangent);
    out.texCoords = vertices[vertexId].texCoords;
    out.NdotT = dot(out.normal, out.tangent);
    return out;
}

struct MaterialTexture
{
    texture2d<float, access::sample> DiffuseTexture [[texture(0)]];
    texture2d<float, access::sample> NormalTexture [[texture(1)]];
    sampler sampler2d [[sampler(0)]];
};

//fragment half4 fragment_lit_textured(VertexOut fragmentIn [[stage_in]],
//                                     texture2d<float, access::sample> DiffuseTexture [[texture(0)]],
//                                     texture2d<float, access::sample> NormalTexture [[texture(1)]],
//                                     sampler sampler2d [[sampler(0)]])
fragment half4 fragment_lit_textured(VertexOut fragmentIn [[stage_in]], MaterialTexture Textures)
{
    //return fragmentIn.texCoords.x;
    //return normalize((half4)float4(fragmentIn.tangent, 1));
    //return dot(normalize(fragmentIn.normal), normalize(fragmentIn.tangent)) * 100.0f;
    
    float3 BumpNormal = Textures.NormalTexture.sample(Textures.sampler2d, fragmentIn.texCoords).rgb * 2.0 - 1.0;
    float3 Binormal = cross(fragmentIn.tangent, fragmentIn.normal);
    Binormal = normalize(Binormal);
    float3 WorldNormal = normalize(fragmentIn.normal) * BumpNormal.z + normalize(fragmentIn.tangent) * BumpNormal.x - Binormal * BumpNormal.y;
    
    // Sample the texture to get the surface color at this point
    half3 surfaceColor = half3(Textures.DiffuseTexture.sample(Textures.sampler2d, fragmentIn.texCoords).rgb);
    // Re-normalize the interpolated surface normal
    half3 normal = normalize(half3(fragmentIn.normal));
    // Compute the ambient color contribution
    half3 color = ambientLightIntensity * surfaceColor;
    // Calculate the diffuse factor as the dot product of the normal and light direction
    float diffuseFactor = saturate(dot(normal, -lightDirection));
    // Add in the diffuse contribution from the light
    color += diffuseFactor * diffuseLightIntensity * surfaceColor;
    
    float diffuseFactorWithBumpTexture = saturate(dot((half3)WorldNormal, -lightDirection));
    return diffuseFactorWithBumpTexture;
    return half4(color, 1);
}
