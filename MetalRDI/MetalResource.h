#pragma once

#include "RenderResource.h"

class MTMetalTypeConverter
{
public:
    static MTLPrimitiveType PrimitiveType(MTPrimitiveType Type)
    {
        switch (Type)
        {
            case MTPrimitiveType::Point :           return MTLPrimitiveTypePoint;
            case MTPrimitiveType::Line :            return MTLPrimitiveTypeLine;
            case MTPrimitiveType::LineStrip :       return MTLPrimitiveTypeLineStrip;
            case MTPrimitiveType::Triangle :        return MTLPrimitiveTypeTriangle;
            case MTPrimitiveType::TriangleStrip :   return MTLPrimitiveTypeTriangleStrip;
        }
        
        return MTLPrimitiveTypeTriangle;
    }
    
    static MTLIndexType IndexType(MTIndexType Type)
    {
        switch (Type)
        {
            case MTIndexType::UInt16 :  return MTLIndexTypeUInt16;
            case MTIndexType::UInt32 :  return MTLIndexTypeUInt32;
        }
        
        return MTLIndexTypeUInt32;
    }

    static MTLCompareFunction CompareFunction(MTCompareFunction CompareFunction)
    {
        switch (CompareFunction)
        {
            case MTCompareFunction::Never :         return MTLCompareFunctionNever;
            case MTCompareFunction::Always :        return MTLCompareFunctionAlways;
            case MTCompareFunction::Equal :         return MTLCompareFunctionEqual;
            case MTCompareFunction::NotEqual :      return MTLCompareFunctionNotEqual;
            case MTCompareFunction::Less :          return MTLCompareFunctionLess;
            case MTCompareFunction::LessEqual :     return MTLCompareFunctionLessEqual;
            case MTCompareFunction::Greater :       return MTLCompareFunctionGreater;
            case MTCompareFunction::GreaterEqual :  return MTLCompareFunctionGreaterEqual;
        }
        
        return MTLCompareFunctionNever;
    }
    
    static MTLWinding Winding(MTTriangleFacingWinding Winding)
    {
        switch (Winding)
        {
            case MTTriangleFacingWinding::Clockwise :           return MTLWindingClockwise;
            case MTTriangleFacingWinding::CounterClockwise :    return MTLWindingCounterClockwise;
        }
        
        return MTLWindingClockwise;
    }
    
    static MTLCullMode CullMode(MTCullMode CullMode)
    {
        switch (CullMode)
        {
            case MTCullMode::None :     return MTLCullModeNone;
            case MTCullMode::Front :    return MTLCullModeFront;
            case MTCullMode::Back :     return MTLCullModeBack;
        }
        
        return MTLCullModeFront;
    }
};

struct MTMetalWindowView : public MTRenderWindowView
{
    CAMetalLayer* MetalLayer = nullptr;
    id<CAMetalDrawable> CurrentMetalDrawable;
    id<MTLTexture> DepthBuffer;
    
    void BeginFrame()
    {
        CurrentMetalDrawable = [MetalLayer nextDrawable];
    }
};

struct MTMetalShader : public MTRenderShader
{
    id<MTLLibrary> Library;
    id<MTLFunction> VertexShaderFunction;
    id<MTLFunction> PixelShaderFunction;
    
    id<MTLRenderPipelineState> RenderPipelineState;
};

struct MTMetalTexture : public MTRenderTexture
{
    id<MTLTexture> Texture;
    id<MTLSamplerState> Sampler;
};

struct MTMetalBuffer : public MTRenderBuffer
{
    id<MTLBuffer> Buffer;
};

struct MTMetalDepthStencilState : MTRenderDepthStencilState
{
    id<MTLDepthStencilState> DepthStencilState;
};
