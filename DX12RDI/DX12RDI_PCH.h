#ifndef DX12RDI_pch
#define DX12RDI_pch

#include "Core.h"
#include "IRDI.h"

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include "d3dcompiler.h"

// DirectXTK12 Header
#include "DescriptorHeap.h"
#include "ResourceUploadBatch.h"
#include "WICTextureLoader.h"
#include "DirectXHelpers.h"

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
using Microsoft::WRL::ComPtr;

#include "DX12RDI.h"
#include "DX12Fence.h"
#include "DX12Resource.h"

#endif /* DX12RDI_pch */
