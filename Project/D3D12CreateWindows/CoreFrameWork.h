#pragma once
#include "stdafx.h"

class Win32Application;

class CoreFrameWork
{
public:
	CoreFrameWork(UINT width, UINT height, std::wstring name);
	virtual ~CoreFrameWork();

	//virtual void OnInit() = 0;
	//virtual void OnUpdate() = 0;
	//virtual void OnRender() = 0;
	//virtual void OnDestroy() = 0;
	virtual void OnInit();
	virtual void OnUpdate() {};
	virtual void OnRender();
	virtual void OnDestroy() {};

	// Samples override the event handlers to handle specific messages.
	virtual void OnKeyDown(UINT8 /*key*/) {}
	virtual void OnKeyUp(UINT8 /*key*/) {}

	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }

	void SetCustomWindowText(LPCWSTR text);

protected:
	// Viewport dimensions.
	UINT m_width;
	UINT m_height;
	float m_aspectRatio;

	// Adapter info.
	bool m_useWarpDevice;

private:
	// Root assets path.
	std::wstring m_assetsPath;

	// Window title.
	std::wstring m_title;

private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
	static const UINT FrameCount = 2;
	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	UINT m_rtvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_frameIndex;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
	HANDLE m_fenceEvent;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

protected:
	void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

public:
	inline ComPtr<ID3D12Device> GetDevices(int _index = 0) {
		return m_device;
	}
};

#include "Win32Application.h"