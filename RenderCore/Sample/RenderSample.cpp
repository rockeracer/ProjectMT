#include "RenderSample.h"
#include "Shader.h"

MTRenderSample::~MTRenderSample()
{
}

void MTRenderSample::Init(IRDI* RDI)
{
	if (RDI)
	{
		m_RDI = RDI;

		m_DepthStencilState = m_RDI->CreateDepthStencilState(MTCompareFunction::LessEqual, true);

		m_CommandBuffer = m_RDI->CreateCommandBuffer();

		//LoadMesh("../../Data/temple.obj", "../../Data/");
	}
}

void MTRenderSample::Destroy()
{
	if (m_DepthStencilState)
	{
		delete m_DepthStencilState;
	}

	if (m_CommandBuffer)
	{
		delete m_CommandBuffer;
	}
}

void MTRenderSample::Render(MTRDIWindowView* View)
{
	if (m_CommandBuffer == nullptr)
	{
		return;
	}

    m_CommandBuffer->BeginCommand(View, false, MTColor(0.0f, 0.2f, 0.4f, 1.0f), false, 1.0f);
    
    MTMatrix4x4 modelToWorldMatrix = MTMatrix4x4(1, 0, 0, 0,
                                                 0, 1, 0, 0,
                                                 0, 0, 1, 0,
                                                 0, 0, 0, 1);
    static float Angle = 0.f;
	//Angle += 0.01f;
    //Angle += 0.003;
    modelToWorldMatrix = MTMatrix4x4::FromRotationMatrix(MTMatrix3x3::RotationY(Angle));
    //matrix4x4_rotation(float(0) * 0.5, vector_float3(0.7, 1, 0));		
    
    // So that the figure doesn't get distorted when the window changes size or rotates,
    // we factor the current aspect ration into our projection matrix. We also select
    // sensible values for the vertical view angle and the distances to the near and far planes.
    //let viewSize = self.view.bounds.size
	float aspectRatio = View->GetAspectRatio();
    float verticalViewAngle = MTMath::DegreeToRadian(65.0f);
    float nearZ = 0.1f;
    float farZ = 100000.0f;
    
    //    matrix_float4x4 projectionMatrix = matrix_perspective(verticalViewAngle, aspectRatio, nearZ, farZ);
    //    matrix_float4x4 viewMatrix = matrix_look_at(0, 0, 2.5, 0, 0, 0, 0, 1, 0);
    //    matrix_float4x4 mvMatrix = viewMatrix;//matrix_multiply(viewMatrix, modelToWorldMatrix);
    
    MTMatrix4x4 projectionMatrix = MTMatrix4x4::Perspective(verticalViewAngle, aspectRatio, nearZ, farZ, -1);
    MTMatrix4x4 viewMatrix = MTMatrix4x4::LookAt(MTVector3(0, 0, 0), MTVector3(0, 800, 1000), MTVector3(0, 1, 0));
    
    // The combined model-view-projection matrix moves our vertices from model space into clip space
	MTMatrix4x4 mvMatrix = viewMatrix * modelToWorldMatrix;
	m_Constants.modelViewProjectionMatrix = (projectionMatrix * mvMatrix).Transpose();
    //m_Constants.modelViewProjectionMatrix = matrix_multiply(projectionMatrix, mvMatrix);
    m_Constants.normalMatrix = MTMatrix4x4::Identity();//matrix_inverse_transpose(matrix_upper_left_3x3(mvMatrix))
    
    
    
    // Since we specified the vertices of our triangles in counter-clockwise
    // order, we need to switch from the default of clockwise winding.
    m_CommandBuffer->SetTriangleFacingWinding(MTTriangleFacingWinding::CounterClockwise);
    m_CommandBuffer->SetCullMode(MTCullMode::Front);
    m_CommandBuffer->SetDepthStencilState(m_DepthStencilState);
//     m_CommandBuffer->SetVertexBuffer(m_Mesh->VertexBuffer);
// //     m_CommandBuffer->SetVertexData(&m_Constants, sizeof(ShaderConstants), 1);
//  	m_CommandBuffer->SetConstantBuffer(&m_Constants, sizeof(ShaderConstants), 1);
//     
//     for (MTInt32 i = 0 ; i< m_Mesh->Submeshes.Num(); ++i)
//     {
//         if (m_Mesh->Materials.IsValidIndex(i))
//         {
// //             MTRenderMaterial* Material = m_Mesh->Materials[i];
// //             for (MTInt32 iTexture = 0 ; iTexture< Array::Size(Material->Textures); ++iTexture)
// //             {
// //                 m_CommandBuffer->SetPixelTexture(Material->Textures[iTexture], iTexture);
// //             }
// 
// 			m_CommandBuffer->SetMaterial(m_Mesh->Materials[i]);
//         }
//         
//         m_CommandBuffer->DrawMesh(m_Mesh, i);
//     }
    
    m_CommandBuffer->EndCommand(View);
}
