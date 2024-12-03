#pragma once

#include <limits>
#include "mathfu/vector.h"
#include "mathfu/matrix.h"

typedef mathfu::Vector<float, 2> MTVector2;
typedef mathfu::Vector<float, 3> MTVector3;
typedef mathfu::Vector<float, 4> MTVector4;

typedef mathfu::Matrix<float, 4, 4> MTMatrix4x4;
typedef mathfu::Matrix<float, 3, 3> MTMatrix3x3;

class CORE_API MTMath
{
public:
	template<typename T>
	static T Min(const T X, const T Y)
	{
		return (X < Y) ? X : Y;
	}

	template<typename T>
	static T Max(const T X, const T Y)
	{
		return (X > Y) ? X : Y;
	}

	template<typename T>
	static T Clamp(const T X, const T Min, const T Max)
	{
		return X < Min ? Min : X < Max ? X : Max;
	}

	static void ComputeVertexTangent(const MTVector3& Pos0, const MTVector3& Pos1, const MTVector3& Pos2,
		const MTVector2& UV0, const MTVector2& UV1, const MTVector2& UV2,
		MTVector3& outTangent, MTVector3& outBinormal);

	static MTVector3 OrthogonalizeVertexTangent(const MTVector3& Normal, const MTVector3& Tangent, const MTVector3& Binormal);

	static float DegreeToRadian(float Degree)
	{
		return (Degree / 180.0f) * (float)M_PI;
	}

	template<typename T>
	static T NumMax()
	{
		return std::numeric_limits<T>::max();
	}

	static MTUInt32 TextureMipCount(MTUInt32 Width, MTUInt32 Height)
	{
		if (Width == 0 || Height == 0)
		{
			return 0;
		}

		MTUInt32 Count = 1;
		while (Width > 1 || Height > 1)
		{
			Width >>= 1;
			Height >>= 1;
			++Count;
		}

		return Count;
	}
};
