#include "stdafx.h"
#include "CoreFrameWork.h"

void CoreFrameWork::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L": " + text;
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}

// Direct3D 12�� �����ϴ� ù ��° ��� ������ �ϵ���� ����͸� ������� ����� �Լ�.
// �׷� ����Ͱ� ������ * ppAdapter�� nullptr�� �����˴ϴ�.
void CoreFrameWork::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// �⺻ ������ ����̹� ����͸� �������� ���ʽÿ�.
			// ����Ʈ���� ����Ͱ� �ʿ��� ��� ��� �ٿ��� "/ warp"�� �����Ͻʽÿ�.
			continue;
		}

		// ����Ͱ� Direct3D 12�� �����ϴ��� Ȯ��������
		// ���� ��ġ�� ���� �����ϴ�.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}

CoreFrameWork::CoreFrameWork(UINT width, UINT height, std::wstring name) :
	m_width(width),
	m_height(height),
	m_title(name),
	m_useWarpDevice(false)
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	m_assetsPath = assetsPath;

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

CoreFrameWork::~CoreFrameWork()
{
}

void CoreFrameWork::OnInit()
{
	/*****************************/
	/* DX12 �ٹ��̽� ���� ���������� */
	/*****************************/

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ����� ���̾� ����
	////////////////////////////////////////////////////////////////////////////////////////////////
	INT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	// ����� ���̾ Ȱ��ȭ�մϴ� (�׷��� ������ "������ ���"�ʿ�).
	// ���� : ��ġ ���� �� ����� ���̾ Ȱ��ȭ�ϸ� Ȱ�� ��ġ�� ��ȿȭ�˴ϴ�.
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		// �߰� ����� ���̾ ����մϴ�.
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���丮 ����
	////////////////////////////////////////////////////////////////////////////////////////////////
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	////////////////////////////////////////////////////////////////////////////////////////////////
	// �ϵ���� ��� ���
	////////////////////////////////////////////////////////////////////////////////////////////////
	ComPtr<IDXGIAdapter1> hardwareAdapter[DX12_DEVICE_COUNT];
	DXGI_ADAPTER_DESC1 desc[DX12_DEVICE_COUNT];
	for (int i=0; i<DX12_DEVICE_COUNT; i++)
	{
		factory->EnumAdapters1(i, &hardwareAdapter[i]);
		hardwareAdapter[i]->GetDesc1(&desc[i]);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// �ϵ���� ����̽� ���
	////////////////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < DX12_DEVICE_COUNT; i++)
	{
		ThrowIfFailed(D3D12CreateDevice(hardwareAdapter[i].Get(), gD3D_FEATURE_LEVEL, IID_PPV_ARGS(&m_device[i])));
		//D3D12CreateDevice(hardwareAdapter[i].Get(), gD3D_FEATURE_LEVEL, _uuidof(ID3D12Device), nullptr);
	}
	ComPtr<ID3D12Device> dsfsf = GetDevices();
	ComPtr<ID3D12Device> dsfsfss = GetDevices(1);
	int iiiiiii = 0;
}