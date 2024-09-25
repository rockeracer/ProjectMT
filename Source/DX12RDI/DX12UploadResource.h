#pragma once

class MTDX12UploadResource
{
public:
	~MTDX12UploadResource()
	{
		if (m_UploadBuffer && m_DynamicBuffer)
		{
			m_UploadBuffer->Unmap(0, nullptr);
		}
	}

	void InitAsStatic(ID3D12Device* Device, D3D12_RESOURCE_DESC* Desc, const void* Data, MTInt32 DataSize, D3D12_RESOURCE_STATES ResourceState)
	{
		m_bGPUUpload = true;
		m_Data = Data;
		m_DataSize = DataSize;
		m_ResourceState = ResourceState;

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			Desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_DefaultBuffer)));
	}

	void InitAsDynamic(ID3D12Device* Device, D3D12_RESOURCE_DESC* Desc)
	{
		m_bGPUUpload = false;

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			Desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_UploadBuffer)));

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_UploadBuffer->Map(0, &readRange, &m_DynamicBuffer));
	}

	void UploadToGPU(DirectX::ResourceUploadBatch& UploadBatch)
	{
		if (m_bGPUUpload && m_Data)
		{
			D3D12_SUBRESOURCE_DATA SrcData = {};
			SrcData.pData = m_Data;
			SrcData.RowPitch = m_DataSize;
			SrcData.SlicePitch = SrcData.RowPitch;

			UploadBatch.Upload(m_DefaultBuffer.Get(), 0, &SrcData, 1);
			UploadBatch.Transition(m_DefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, m_ResourceState);
		}
	}

	void UpdateBuffer(void* Data, MTInt32 Size)
	{
		if ((m_bGPUUpload == false) && m_DynamicBuffer && Data && (Size > 0))
		{
			memcpy(m_DynamicBuffer, Data, Size);
		}
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUBufferLocation() const
	{
		MT_ASSERT(m_DefaultBuffer);
		return m_DefaultBuffer->GetGPUVirtualAddress();
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetUploadGPUBufferLocation() const
	{
		MT_ASSERT(m_UploadBuffer);
		return m_UploadBuffer->GetGPUVirtualAddress();
	}

	MTInt32 GetDataSize() const
	{
		return m_DataSize;
	}

private:
	bool m_bGPUUpload = false;
	
	const void* m_Data = nullptr;
	MTInt32 m_DataSize = 0;
	D3D12_RESOURCE_STATES m_ResourceState;

	ComPtr<ID3D12Resource> m_DefaultBuffer;
	ComPtr<ID3D12Resource> m_UploadBuffer;
	void* m_DynamicBuffer = nullptr;
};
