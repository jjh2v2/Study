#include "stdafx.h"
#include "CoreFrameWork.h"


using namespace Microsoft::WRL;

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
	// �ϵ���� ��� ��� ����
	////////////////////////////////////////////////////////////////////////////////////////////////
	ComPtr<IDXGIAdapter1> hardwareAdapter0, hardwareAdapter1;
	factory->EnumAdapters1(0, &hardwareAdapter0);
	factory->EnumAdapters1(1, &hardwareAdapter1);

	DXGI_ADAPTER_DESC1 desc0, desc1;
	hardwareAdapter0->GetDesc1(&desc0);
	hardwareAdapter1->GetDesc1(&desc1);
	
	ComPtr<ID3D12Device> m_device0, m_device1;

	HRESULT hr = S_OK;
	hr = D3D12CreateDevice(hardwareAdapter0.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device0));
	hr = D3D12CreateDevice(hardwareAdapter1.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device1));
	//D3D12CreateDevice(hardwareAdapter0.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);
	//D3D12CreateDevice(hardwareAdapter1.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);
	if (hr == S_FALSE || hr == S_OK) {
		// �������� ����̽��� ��������� S_FALSE=0  S_OK=1 ���� ����̽��� ��������� �ʴ´ٸ� 0���� ���� ���� ���´�
	}

	int iiiiiii = 0;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter0);
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

void CoreFrameWork::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L": " + text;
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}