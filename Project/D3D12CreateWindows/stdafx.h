
#pragma once

// Link library.
 #pragma comment(lib, "DXGI.lib")
 #pragma comment(lib, "D3DCompiler.lib")
 #pragma comment(lib, "D3D12.lib")
 #pragma comment(lib, "D2D1.lib")
 #pragma comment(lib, "DWrite.lib")
 #pragma comment(lib, "WindowsCodecs.lib")
 #pragma comment(lib, "Strmiids.lib")
 #pragma comment(lib, "DSound.lib")
 //#pragma comment(lib, "DShow.lib")
 #pragma comment(lib, "DInput8.lib")
 #pragma comment(lib, "DXGuid.lib")

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
{
	if (path == nullptr)
	{
		throw std::exception();
	}

	DWORD size = GetModuleFileName(nullptr, path, pathSize);
	if (size == 0 || size == pathSize)
	{
		// Method failed or path was truncated.
		throw std::exception();
	}

	WCHAR* lastSlash = wcsrchr(path, L'\\');
	if (lastSlash)
	{
		*(lastSlash + 1) = L'\0';
	}
}