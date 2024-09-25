#pragma once

#include "RDIResource.h"
#include "DX12UploadResource.h"

class IDX12UploadResource
{
public:
	virtual void Upload(ID3D12Device* Device, DirectX::ResourceUploadBatch& UploadBatch) = 0;
};

class MTDX12TypeConverter
{
public:
//     static MTLPrimitiveType PrimitiveType(MTPrimitiveType Type)
//     {
//         switch (Type)
//         {
//             case MTPrimitiveType::Point :           return MTLPrimitiveTypePoint;
//             case MTPrimitiveType::Line :            return MTLPrimitiveTypeLine;
//             case MTPrimitiveType::LineStrip :       return MTLPrimitiveTypeLineStrip;
//             case MTPrimitiveType::Triangle :        return MTLPrimitiveTypeTriangle;
//             case MTPrimitiveType::TriangleStrip :   return MTLPrimitiveTypeTriangleStrip;
//         }
//         
//         return MTLPrimitiveTypeTriangle;
//     }
//     
//     static MTLIndexType IndexType(MTIndexType Type)
//     {
//         switch (Type)
//         {
//             case MTIndexType::UInt16 :  return MTLIndexTypeUInt16;
//             case MTIndexType::UInt32 :  return MTLIndexTypeUInt32;
//         }
//         
//         return MTLIndexTypeUInt32;
//     }
// 
//     static MTLCompareFunction CompareFunction(MTCompareFunction CompareFunction)
//     {
//         switch (CompareFunction)
//         {
//             case MTCompareFunction::Never :         return MTLCompareFunctionNever;
//             case MTCompareFunction::Always :        return MTLCompareFunctionAlways;
//             case MTCompareFunction::Equal :         return MTLCompareFunctionEqual;
//             case MTCompareFunction::NotEqual :      return MTLCompareFunctionNotEqual;
//             case MTCompareFunction::Less :          return MTLCompareFunctionLess;
//             case MTCompareFunction::LessEqual :     return MTLCompareFunctionLessEqual;
//             case MTCompareFunction::Greater :       return MTLCompareFunctionGreater;
//             case MTCompareFunction::GreaterEqual :  return MTLCompareFunctionGreaterEqual;
//         }
//         
//         return MTLCompareFunctionNever;
//     }
//     
//     static MTLWinding Winding(MTTriangleFacingWinding Winding)
//     {
//         switch (Winding)
//         {
//             case MTTriangleFacingWinding::Clockwise :           return MTLWindingClockwise;
//             case MTTriangleFacingWinding::CounterClockwise :    return MTLWindingCounterClockwise;
//         }
//         
//         return MTLWindingClockwise;
//     }
//     
//     static MTLCullMode CullMode(MTCullMode CullMode)
//     {
//         switch (CullMode)
//         {
//             case MTCullMode::None :     return MTLCullModeNone;
//             case MTCullMode::Front :    return MTLCullModeFront;
//             case MTCullMode::Back :     return MTLCullModeBack;
//         }
//         
//         return MTLCullModeFront;
//     }
};

class MTDX12WindowView : public MTRDIWindowView
{
public:
	MTDX12WindowView(MTUInt32 Width, MTUInt32 Height) : MTRDIWindowView(Width, Height)
	{}

	virtual ~MTDX12WindowView()
	{
		CloseHandle(m_SwapChainWaitableObject);
	}

	virtual void OnResize(MTUInt32 Width, MTUInt32 Height) override
	{
		if (m_Device && m_SwapChain)
		{
			m_SwapChain->ResizeBuffers(m_FrameCount, Width, Height,
				DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);

			CreateRenderTarget(m_Device, Width, Height);
		}
	}

	void Init(ID3D12Device* Device, IDXGIFactory4* GIFactory, ID3D12CommandQueue* CommandQueue,
		HWND hWnd, MTUInt32 Width, MTUInt32 Height, MTInt32 BackbufferCount)
	{
		ThrowIfFailed(BackbufferCount > MaxFrameCount);

		m_Device = Device;

		m_FrameCount = BackbufferCount;

		// Create SwapChain
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.BufferCount = BackbufferCount;
			swapChainDesc.Width = Width;
			swapChainDesc.Height = Height;
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.SampleDesc.Count = 1;

			ComPtr<IDXGISwapChain1> SwapChain;
			ThrowIfFailed(GIFactory->CreateSwapChainForHwnd(
				CommandQueue,		// Swap chain needs the queue so that it can force a flush on it.
				hWnd,
				&swapChainDesc,
				nullptr,
				nullptr,
				&SwapChain
			));

			ThrowIfFailed(SwapChain.As(&m_SwapChain));
			m_SwapChain->SetMaximumFrameLatency(BackbufferCount);
			m_SwapChainWaitableObject = m_SwapChain->GetFrameLatencyWaitableObject();

			m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
		}


		MTInt32 rtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = BackbufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (MTInt32 i = 0; i < BackbufferCount; ++i)
		{
			m_RenderTargetDescriptor[i] = rtvHandle;
			rtvHandle.Offset(1, rtvDescriptorSize);
		}


		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));


		CreateRenderTarget(Device, Width, Height);
	}

	void Present()
	{
		if (m_SwapChain)
		{
			ThrowIfFailed(m_SwapChain->Present(1, 0));
			m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
		}
	}

	void BeginRender(ID3D12GraphicsCommandList* CommandList, const float ClearColor[4], float ClearDepth)
	{
		WaitForSingleObject(m_SwapChainWaitableObject, INFINITE);

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RenderTargetDescriptor[m_FrameIndex];
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		CommandList->ClearRenderTargetView(rtvHandle, ClearColor, 0, nullptr);
		CommandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, ClearDepth, 0, 0, nullptr);

		D3D12_VIEWPORT vp;
		memset(&vp, 0, sizeof(D3D12_VIEWPORT));
		vp.Width = (float)GetWidth();
		vp.Height = (float)GetHeight();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = vp.TopLeftY = 0.0f;
		CommandList->RSSetViewports(1, &vp);

		const D3D12_RECT ScissorRects = { 0, 0, (LONG)(GetWidth()), (LONG)(GetHeight()) };
		CommandList->RSSetScissorRects(1, &ScissorRects);
	}

	void EndRender(ID3D12GraphicsCommandList* CommandList)
	{
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	}

private:
	void CreateRenderTarget(ID3D12Device* Device, MTInt32 Width, MTInt32 Height)
	{
		// Create RenderTargetView
		{
			for (MTInt32 i = 0; i < m_FrameCount; ++i)
			{
				m_RenderTargets[i].Reset();

				ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i])));
				Device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, m_RenderTargetDescriptor[i]);
			}
		}

		// Create DepthStencilView
		{
			m_DepthStencil.Reset();

			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
			depthOptimizedClearValue.DepthStencil.Stencil = 0;

			ThrowIfFailed(Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(&m_DepthStencil)
			));

			Device->CreateDepthStencilView(m_DepthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
	}

	static const MTUInt32 MaxFrameCount = 3;

	ID3D12Device* m_Device = nullptr;

	ComPtr<IDXGISwapChain3> m_SwapChain;
	MTInt32 m_FrameCount = MaxFrameCount;
	MTUInt32 m_FrameIndex = 0;
	HANDLE m_SwapChainWaitableObject = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	
	D3D12_CPU_DESCRIPTOR_HANDLE m_RenderTargetDescriptor[MaxFrameCount] = {};
	ComPtr<ID3D12Resource> m_RenderTargets[MaxFrameCount] = {};
	ComPtr<ID3D12Resource> m_DepthStencil;
};

class MTDX12Shader : public MTRDIShader
{
public:
	ComPtr<ID3DBlob> VertexShader;
	ComPtr<ID3DBlob> PixelShader;
	ComPtr<ID3D12PipelineState> PipelineState;
};

class MTDX12Texture : public MTRDITexture, public IDX12UploadResource
{
public:
	MTString FilePath;
	MTUChar* TextureData = nullptr;
	MTUInt32 TextureDataSize = 0;
	MTUInt32 Width = 0;
	MTUInt32 Height = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor;

	virtual void Upload(ID3D12Device* Device, DirectX::ResourceUploadBatch& UploadBatch) override
	{
		if (!FilePath.IsEmpty())
		{
			size_t newsize = FilePath.Length() + 1;
			wchar_t* wcstring = new wchar_t[newsize];
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, wcstring, newsize, FilePath.c_str(), _TRUNCATE);

			ThrowIfFailed(DirectX::CreateWICTextureFromFile(Device, UploadBatch, wcstring, &m_Resource, true));
		}
		else if (TextureData && (TextureDataSize > 0))
		{
			D3D12_SUBRESOURCE_DATA SubResource;

			D3D12_RESOURCE_DESC Desc = {};
			Desc.Width = Width;
			Desc.Height = Height;
			Desc.MipLevels = MTMath::TextureMipCount(Width, Height);
			Desc.DepthOrArraySize = 1;
			Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			Desc.SampleDesc.Count = 1;
			Desc.SampleDesc.Quality = 0;
			Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
			Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

			HRESULT hr = Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&Desc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_GRAPHICS_PPV_ARGS(&m_Resource));

			ThrowIfFailed(hr);

			SubResource.pData = TextureData;
			SubResource.RowPitch = TextureDataSize / Height;
			SubResource.SlicePitch = TextureDataSize;

			UploadBatch.Upload(m_Resource.Get(), 0, &SubResource, 1);

			UploadBatch.Transition(m_Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			UploadBatch.GenerateMips(m_Resource.Get());

			delete[] TextureData;
		}
		else
		{
			MT_ASSERT_EXPR(0, "Dx12 texture isn't valid.");
		}
	}

	bool CreateShaderResourceView(ID3D12Device* Device, D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor)
	{
		if (m_Resource)
		{
			DirectX::CreateShaderResourceView(Device, m_Resource.Get(), srvDescriptor);
			return true;
		}

		return false;
	}

private:
	ComPtr<ID3D12Resource> m_Resource;
};

class  MTDX12VertexBuffer : public MTRDIBuffer, public IDX12UploadResource
{
public:
	MTDX12VertexBuffer(MTUInt32 Size, MTUInt32 Stride) : MTRDIBuffer(Size, Stride)
	{}

	void Init(ID3D12Device* Device, const void* Data, MTInt32 DataSize, MTInt32 VertexSize)
	{
		m_VertexSize = VertexSize;
		m_UploadBuffer.InitAsStatic(Device, &CD3DX12_RESOURCE_DESC::Buffer(DataSize), Data, DataSize, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	virtual void Upload(ID3D12Device* Device, DirectX::ResourceUploadBatch& UploadBatch) override
	{
		m_UploadBuffer.UploadToGPU(UploadBatch);

		m_VertexBufferView.BufferLocation = m_UploadBuffer.GetGPUBufferLocation();
		m_VertexBufferView.SizeInBytes = m_UploadBuffer.GetDataSize();
		m_VertexBufferView.StrideInBytes = m_VertexSize;
	}

	const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const
	{
		return &m_VertexBufferView;
	}

private:
	MTDX12UploadResource m_UploadBuffer;
	
	MTInt32 m_VertexSize;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
};

class MTDX12IndexBuffer : public MTRDIBuffer, public IDX12UploadResource
{
public:
	MTDX12IndexBuffer(MTUInt32 Size, MTUInt32 Stride) : MTRDIBuffer(Size, Stride)
	{}

	void Init(ID3D12Device* Device, const void* Data, MTInt32 DataSize, MTInt32 IndexSize)
	{
		m_IndexSize = IndexSize;
		m_UploadBuffer.InitAsStatic(Device, &CD3DX12_RESOURCE_DESC::Buffer(DataSize), Data, DataSize, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	virtual void Upload(ID3D12Device* Device, DirectX::ResourceUploadBatch& UploadBatch) override
	{
		m_UploadBuffer.UploadToGPU(UploadBatch);

		m_IndexBufferView.BufferLocation = m_UploadBuffer.GetGPUBufferLocation();
		m_IndexBufferView.SizeInBytes = m_UploadBuffer.GetDataSize();
		switch (m_IndexSize)
		{
		case 1:
			m_IndexBufferView.Format = DXGI_FORMAT_R8_UINT;
			break;
		case 2:
			m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
			break;
		case 4:
			m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
			break;
		default:
			MT_ASSERT_EXPR(0, "Index Size not supported.");
		}
	}

	const D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() const
	{
		return &m_IndexBufferView;
	}

private:
	MTDX12UploadResource m_UploadBuffer;

	MTInt32 m_IndexSize;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};

class MTDX12DepthStencilState : MTRDIDepthStencilState
{
public:
//    id<MTLDepthStencilState> DepthStencilState;
};

class MTDX12Material : public MTRDIMaterial
{
public:
	ComPtr<ID3D12PipelineState> PipelineState;

	void Init(const MTString& VertexShaderCode, const MTString& PixelShaderCode, ID3D12Device* Device, ID3D12RootSignature* RootSignature, const MTArray<MTRDITexture*> Textures)
	{
		CreateDescriptorHeap(Device, Textures.Num());
		CreatePipelineState(VertexShaderCode, PixelShaderCode, Device, RootSignature);
		CreateSampler(Device);

		m_Textures = Textures;
	}

	void Commit(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
	{
		SetShaderResourceView(Device);

		CommandList->SetPipelineState(PipelineState.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { m_DescriptorHeap_CBV_SRV->Heap(), m_DescriptorHeap_Sampler->Heap() };
		CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		CommandList->SetGraphicsRootDescriptorTable(0, m_DescriptorHeap_CBV_SRV->GetGpuHandle(0));
		CommandList->SetGraphicsRootDescriptorTable(3, m_DescriptorHeap_Sampler->GetGpuHandle(0));
	}

private:
	void CreateDescriptorHeap(ID3D12Device* Device, MTInt32 TextureCount)
	{
		m_DescriptorHeap_CBV_SRV = std::make_unique<DirectX::DescriptorHeap>(Device,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			TextureCount);

		m_DescriptorHeap_Sampler = std::make_unique<DirectX::DescriptorHeap>(Device,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			TextureCount);
	}

	void CreatePipelineState(const MTString& VertexShaderCode, const MTString& PixelShaderCode, ID3D12Device* Device, ID3D12RootSignature* RootSignature)
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT CompileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		D3D_SHADER_MACRO Shader_Macros[] = { "DX12", "1", nullptr, nullptr };

		ComPtr<ID3DBlob> VertexShader;
		ComPtr<ID3DBlob> PixelShader;

		ID3DBlob* Error;
		HRESULT hr = D3DCompile(VertexShaderCode.c_str(), VertexShaderCode.Length(), "../../Shader/", Shader_Macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vertex_transform", "vs_5_0", CompileFlags, 0, &VertexShader, &Error);
		if (FAILED(hr))
		{
			char* ErrorMsg = (char*)Error->GetBufferPointer();
			MT_LOG(ErrorMsg);
			throw HrException(hr);
		}

		hr = D3DCompile(PixelShaderCode.c_str(), PixelShaderCode.Length(), "../../Shader/", Shader_Macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "fragment_lit_textured", "ps_5_0", CompileFlags, 0, &PixelShader, &Error);
		if (FAILED(hr))
		{
			char* ErrorMsg = (char*)Error->GetBufferPointer();
			MT_LOG(ErrorMsg);
			throw HrException(hr);
		}

		const D3D12_INPUT_ELEMENT_DESC StandardVertexDescription[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
		inputLayoutDesc.pInputElementDescs = StandardVertexDescription;
		inputLayoutDesc.NumElements = 4;

		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		depthStencilDesc.StencilEnable = FALSE;

		// Describe and create the PSO for rendering the scene.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = inputLayoutDesc;
		psoDesc.pRootSignature = RootSignature;
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = depthStencilDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PipelineState)));
	}

	void CreateSampler(ID3D12Device* Device)
	{
		D3D12_SAMPLER_DESC wrapSamplerDesc = {};
		wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		wrapSamplerDesc.MinLOD = 0;
		wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		wrapSamplerDesc.MipLODBias = 0.0f;
		wrapSamplerDesc.MaxAnisotropy = 1;
		wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
		Device->CreateSampler(&wrapSamplerDesc, m_DescriptorHeap_Sampler->GetCpuHandle(0));
		Device->CreateSampler(&wrapSamplerDesc, m_DescriptorHeap_Sampler->GetCpuHandle(1));
	}

	void SetShaderResourceView(ID3D12Device* Device)
	{
		if (m_bCompleteCreateShaderResourceView == false)
		{
			m_bCompleteCreateShaderResourceView = true;

			MTInt32 Index = 0;
			for (MTRDITexture* Texture : m_Textures)
			{
				MTDX12Texture* DX12Texture = dynamic_cast<MTDX12Texture*>(Texture);
				if (DX12Texture)
				{
					m_bCompleteCreateShaderResourceView &= DX12Texture->CreateShaderResourceView(Device, m_DescriptorHeap_CBV_SRV->GetCpuHandle(Index++));
				}
			}
		}
	}

	std::unique_ptr<DirectX::DescriptorHeap> m_DescriptorHeap_CBV_SRV;
	std::unique_ptr<DirectX::DescriptorHeap> m_DescriptorHeap_Sampler;

	MTArray<MTRDITexture*> m_Textures;
	bool m_bCompleteCreateShaderResourceView = false;
};
