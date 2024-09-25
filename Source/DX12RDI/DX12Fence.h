#pragma once

class MTDX12Fence
{
public:
	void Init(ID3D12Device* DX12Device)
	{
		ThrowIfFailed(DX12Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
		++m_FenceValue;

		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	void Wait(UINT64 FenceValue)
	{
		const UINT64 LastCompletedFence = m_Fence->GetCompletedValue();
		if ((FenceValue > 0) && (FenceValue > LastCompletedFence))
		{
			ThrowIfFailed(m_Fence->SetEventOnCompletion(FenceValue, m_FenceEvent));
			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	UINT64 Signal(const ComPtr<ID3D12CommandQueue>& CommandQueue)
	{
		UINT64 SignalValue = m_FenceValue;
		CommandQueue->Signal(m_Fence.Get(), m_FenceValue);
		++m_FenceValue;
		return SignalValue;
	}

private:
	HANDLE m_FenceEvent;
	ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValue = 0;
};
