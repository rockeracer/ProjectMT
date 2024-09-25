#include "DX12ResourceUploader.h"

MTDX12ResourceUploader::MTDX12ResourceUploader()
{
	m_BufferCount = MTRenderVar::RENDER_FRAME_COUNT;
	m_ReservedResource = new MTArray<IDX12UploadResource*>[m_BufferCount];
}

MTDX12ResourceUploader::~MTDX12ResourceUploader()
{
	delete[] m_ReservedResource;
}

void MTDX12ResourceUploader::ReserveUpload(IDX12UploadResource* Resource)
{
	m_ReservedResource[m_WriteBufferIndex].Add(Resource);
}

void MTDX12ResourceUploader::Flush()
{
	m_ReadBufferIndex = m_WriteBufferIndex;
	m_WriteBufferIndex = (m_WriteBufferIndex + 1) % m_BufferCount;
}

void MTDX12ResourceUploader::Upload(ID3D12Device* Device, ID3D12CommandQueue* CommandQueue)
{
	MT_ASSERT(m_ReadBufferIndex != m_WriteBufferIndex);

	DirectX::ResourceUploadBatch UploadBatch(Device);
	UploadBatch.Begin();
	{
		for (IDX12UploadResource* Resource : m_ReservedResource[m_ReadBufferIndex])
		{
			Resource->Upload(Device, UploadBatch);
		}
	}
	UploadBatch.End(CommandQueue).wait();

	m_ReservedResource[m_ReadBufferIndex].Clear();
}
