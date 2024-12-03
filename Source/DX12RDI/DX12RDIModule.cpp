#include "DX12RDIModule.h"
#include "DX12Resource.h"
#include "DX12CommandBuffer.h"
#include "DX12GUI.h"

MTDX12RDI::MTDX12RDI()
{
}

MTDX12RDI::~MTDX12RDI()
{
}

void MTDX12RDI::Init()
{
	CreateDevice();
	CreateCommandQueue();
	CreateCommandList();
	CreateFence();
	CreateRootSignature();

	m_GraphicsMemory = std::make_unique<DirectX::GraphicsMemory>(m_DX12Device.Get());

#if 0//(_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
	Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
	ThrowIfFailed(initialize);
#else
	ThrowIfFailed(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));
#endif
}

void MTDX12RDI::Destroy()
{
    
}

MTRDIWindowView* MTDX12RDI::CreateWindowView(void* Handle, MTUInt32 Width, MTUInt32 Height, MTInt32 BackbufferCount)
{
	MTDX12WindowView* WindowView = new MTDX12WindowView(Width, Height);
	WindowView->Init(m_DX12Device.Get(), m_GIFactory.Get(), m_CommandQueue.Get(), (HWND)Handle, Width, Height, BackbufferCount);

	return WindowView;
}

void MTDX12RDI::CreateDevice()
{
	UINT dxgiFactoryFlags = 0;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_GIFactory)));

	ComPtr<IDXGIAdapter1> HardwareAdapter;
	GetHardwareAdapter(m_GIFactory.Get(), &HardwareAdapter);

	ThrowIfFailed(D3D12CreateDevice(HardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_DX12Device)));
}

void MTDX12RDI::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_DX12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));
}

void MTDX12RDI::CreateCommandList()
{
	ThrowIfFailed(m_DX12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
	ThrowIfFailed(m_DX12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
	ThrowIfFailed(m_CommandList->Close());
}

void MTDX12RDI::CreateFence()
{
	m_Fence.Init(m_DX12Device.Get());
}

void MTDX12RDI::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_DX12Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[4]; // Perfomance TIP: Order from most frequent to least frequent.
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);	// 2 frequently changed diffuse + normal textures - using registers t1 and t2.
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);	// 1 frequently changed constant buffer.
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);												// 1 infrequently changed shadow texture - starting in register t0.
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);											// 2 static samplers.

	CD3DX12_ROOT_PARAMETER1 rootParameters[4];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	//rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(m_DX12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
}

void MTDX12RDI::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}

void* MTDX12RDI::InitGUIRender(const MTGUIFontTextureDesc& FontTextureDesc, const MTGUIVertexDesc& VertexDesc, MTInt32 RenderFrameCount)
{
	m_DX12GUI = new MTDX12GUI();
	return m_DX12GUI->Init(m_DX12Device, m_CommandList, RenderFrameCount, FontTextureDesc, VertexDesc);
}

void MTDX12RDI::ShutdownGUIRender()
{
	if (m_DX12GUI)
	{
		m_DX12GUI->Destroy();

		delete m_DX12GUI;
		m_DX12GUI = nullptr;
	}
}

void MTDX12RDI::CommitRenderCommandBuffer()
{
	m_ResourceUploader.Flush();
}

void MTDX12RDI::BeginRender()
{
	WaitForLastFrameCompleted();

	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	m_ResourceUploader.Upload(m_DX12Device.Get(), m_CommandQueue.Get());

	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void MTDX12RDI::WaitForLastFrameCompleted()
{
	m_Fence.Wait(m_FenceValue);
}

void MTDX12RDI::RenderGUI(MTRDIWindowView* View, ImDrawData* DrawData)
{
	if (m_DX12GUI)
	{
		m_DX12GUI->RenderGUI(View, DrawData);
	}
}

void MTDX12RDI::EndRender(MTRDIWindowView* View)
{
	MTDX12WindowView* DX12View = dynamic_cast<MTDX12WindowView*>(View);
	if (DX12View)
	{
		DX12View->EndRender(m_CommandList.Get());

		ThrowIfFailed(m_CommandList->Close());

		ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		DX12View->Present();

		m_GraphicsMemory->Commit(m_CommandQueue.Get());

		m_FenceValue = m_Fence.Signal(m_CommandQueue);
	}
}

void MTDX12RDI::WaitForRenderCompleted()
{
	WaitForLastFrameCompleted();
}

MTRDIShader* MTDX12RDI::CreateShaderWithSource(const MTString& Source, const MTString& VertexShaderFunction, const MTString& PixelShaderFunction)
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT CompileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	D3D_SHADER_MACRO Shader_Macros[] = { "DX12", "1", nullptr, nullptr };

	MTDX12Shader* DX12Shader = new MTDX12Shader;
	ID3DBlob* Error;
	HRESULT hr = D3DCompile(Source.c_str(), Source.Length(), nullptr, Shader_Macros, nullptr, VertexShaderFunction.c_str(), "vs_5_0", CompileFlags, 0, &DX12Shader->VertexShader, &Error);
	if (FAILED(hr))
	{
		char* ErrorMsg = (char*)Error->GetBufferPointer();
		MT_LOG(ErrorMsg);
		throw HrException(hr);
	}
	
	hr = D3DCompile(Source.c_str(), Source.Length(), nullptr, Shader_Macros, nullptr, PixelShaderFunction.c_str(), "ps_5_0", CompileFlags, 0, &DX12Shader->PixelShader, &Error);
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
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(DX12Shader->VertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(DX12Shader->PixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	ThrowIfFailed(m_DX12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&DX12Shader->PipelineState)));

	return DX12Shader;
}

IRDICommandBuffer* MTDX12RDI::CreateCommandBuffer()
{
    return new MTDX12CommandBuffer(m_DX12Device.Get(), m_CommandList.Get(), m_GraphicsMemory.get());
}

MTRDIBuffer* MTDX12RDI::CreateVertexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 VertexSize)
{
	MTDX12VertexBuffer* Buffer = new MTDX12VertexBuffer(BufferSize, VertexSize);
	Buffer->Init(m_DX12Device.Get(), Data, BufferSize, VertexSize);
	m_ResourceUploader.ReserveUpload(Buffer);
	return Buffer;
}

MTRDIBuffer* MTDX12RDI::CreateIndexBuffer(const void* Data, MTInt32 BufferSize, MTInt32 IndexSize)
{
	MTDX12IndexBuffer* Buffer = new MTDX12IndexBuffer(BufferSize, IndexSize);
	Buffer->Init(m_DX12Device.Get(), Data, BufferSize, IndexSize);
	m_ResourceUploader.ReserveUpload(Buffer);
	return Buffer;
}

MTRDIDepthStencilState* MTDX12RDI::CreateDepthStencilState(MTCompareFunction CompareFunction, bool bDepthWrite)
{
	return nullptr;
}

MTRDITexture* MTDX12RDI::CreateTextureFromFilePath(const MTString& FilePath)
{
	MTDX12Texture* Texture = new MTDX12Texture;
	Texture->FilePath = FilePath;
	m_ResourceUploader.ReserveUpload(Texture);
	return Texture;
}

MTRDITexture* MTDX12RDI::CreateTextureFromMemory(MTUInt32 Width, MTUInt32 Height, MTUChar* Pixels, MTUInt32 Size)
{
	MTDX12Texture* Texture = new MTDX12Texture;
	Texture->Width = Width;
	Texture->Height = Height;
	Texture->TextureData = new MTUChar[Size];
	memcpy(Texture->TextureData, Pixels, Size);
	Texture->TextureDataSize = Size;
	m_ResourceUploader.ReserveUpload(Texture);
	return Texture;
}

MTRDIMaterial* MTDX12RDI::CreateMaterial(const MTString& VertexShaderCode, const MTString& PixelShaderCode, const MTArray<MTRDITexture*> Textures)
{
	MTDX12Material* Material = new MTDX12Material;
	Material->Init(VertexShaderCode, PixelShaderCode, m_DX12Device.Get(), m_RootSignature.Get(), Textures);
	return Material;
}
