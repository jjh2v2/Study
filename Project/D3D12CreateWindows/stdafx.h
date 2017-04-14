
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
#include <iostream>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <sstream>
#include <atlbase.h>

#include <vector>
#include <list>
#include <unordered_map>

using Microsoft::WRL::ComPtr;
using namespace std;

/*
�콬�� ���¹�
std::unordered_map <char, int> hm1;
hm1.insert(std::unordered_map<int, int>::value_type('a', 1));
*/

const static D3D_FEATURE_LEVEL gD3D_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_1;// D3D_FEATURE_LEVEL_11_0 ~ D3D_FEATURE_LEVEL_12_1
#define DX12_DEVICE_COUNT 2

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

// ������� ���� ���� ��ü�� �̸��� �����մϴ�.
#if defined(_DEBUG)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
	WCHAR fullName[50];
	if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
	{
		pObject->SetName(fullName);
	}
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{
}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
{
}
#endif

// ComPtr <T>�� ���� ��� �����.
// ������ �̸��� ��ü�� �̸����� �����մϴ�.
// �ε��� �� ������ ��ü �̸��� �ε����� �����մϴ�.
#define NAME_D3D12_OBJECT(x) SetName(x.Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed(x[n].Get(), L#x, n)