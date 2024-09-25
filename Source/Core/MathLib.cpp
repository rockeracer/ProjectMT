#include "MathLib.h"

void MTMath::ComputeVertexTangent(const MTVector3& Pos0, const MTVector3& Pos1, const MTVector3& Pos2,
                                  const MTVector2& UV0, const MTVector2& UV1, const MTVector2& UV2,
                                  MTVector3& outTangent, MTVector3& outBinormal)
{
    MTVector3 PosEdge0 = Pos1 - Pos0;
    MTVector3 PosEdge1 = Pos2 - Pos0;
    
    MTVector2 UVEdge0 = UV1 - UV0;
    MTVector2 UVEdge1 = UV2 - UV0;
 
    float InverseScale = UVEdge0.x * UVEdge1.y - UVEdge0.y * UVEdge1.x;
    if (InverseScale != 0.0f)
    {
        float Scale = 1.0f / InverseScale;
        
        outTangent = (PosEdge0 * UVEdge1.y) - (PosEdge1 * UVEdge0.y);
        outTangent *= Scale;
        
        outBinormal = (PosEdge1 * UVEdge0.x) - (PosEdge0 * UVEdge1.x);
        outBinormal *= Scale;
    }
    else
    {
        outTangent = MTVector3(0.0f, 0.0f, 0.0f);
        outBinormal = MTVector3(0.0f, 0.0f, 0.0f);
    }
}

MTVector3 MTMath::OrthogonalizeVertexTangent(const MTVector3& Normal, const MTVector3& Tangent, const MTVector3& Binormal)
{
    
    //tangent[a] = (t - n * Dot(n, t)).Normalize();
    
    // Calculate handedness
    //tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
    
    MTVector3 Result = Tangent - (Normal * MTVector3::DotProduct(Normal, Tangent));
    Result.Normalize();
    
//    Result *= MTVector3::DotProduct(MTVector3::CrossProduct(Tangent, Normal), Binormal) < 0.0f ? -1.0f : 1.0f;
    
    return Result;
}
