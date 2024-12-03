#pragma once

#include "IRDI.h"
#include "RDIResource.h"

class MTRenderSample
{
public:
	~MTRenderSample();

	void Init(IRDI* RDI);
	void Destroy();

	void Render(MTRDIWindowView* View);

private:
	IRDI* m_RDI = nullptr;

	MTRDIDepthStencilState* m_DepthStencilState = nullptr;

	IRDICommandBuffer* m_CommandBuffer = nullptr;

	struct ShaderConstants
	{
		//matrix_float4x4
		MTMatrix4x4 modelViewProjectionMatrix;
		//matrix_float3x3
		MTMatrix4x4 normalMatrix;
	};
	ShaderConstants m_Constants;
};
