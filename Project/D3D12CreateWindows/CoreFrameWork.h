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
	virtual void OnRender() {};
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
	ComPtr<ID3D12Device> m_device[DX12_DEVICE_COUNT];
	ComPtr<ID3D12CommandQueue> m_commandQueue[DX12_DEVICE_COUNT];

protected:
	void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

public:
	inline ComPtr<ID3D12Device> GetDevices(int _index=0) {
		return m_device[_index];
	}
};

#include "Win32Application.h"