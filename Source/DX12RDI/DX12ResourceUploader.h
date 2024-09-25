#pragma once

class IDX12UploadResource;

class MTDX12ResourceUploader
{
public:
	MTDX12ResourceUploader();
	~MTDX12ResourceUploader();

	void ReserveUpload(IDX12UploadResource* Resource);

	void Flush();

	void Upload(ID3D12Device* Device, ID3D12CommandQueue* CommandQueue);

private:
	MTInt32 m_BufferCount = 0;
	MTInt32 m_WriteBufferIndex = 0;
	MTInt32 m_ReadBufferIndex = 0;

	MTArray<IDX12UploadResource*>* m_ReservedResource = nullptr;
};
