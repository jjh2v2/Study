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
	m_frameIndex(0),
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
	ThrowIfFailed( CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)) );




	////////////////////////////////////////////////////////////////////////////////////////////////
	// �ϵ���� ��� ���
	////////////////////////////////////////////////////////////////////////////////////////////////
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	DXGI_ADAPTER_DESC1 desc;
	ThrowIfFailed(factory->EnumAdapters1(0, &hardwareAdapter));
	hardwareAdapter->GetDesc1(&desc);
	



	////////////////////////////////////////////////////////////////////////////////////////////////
	// �ϵ���� ����̽� ���
	////////////////////////////////////////////////////////////////////////////////////////////////
	ThrowIfFailed( D3D12CreateDevice(hardwareAdapter.Get(), gD3D_FEATURE_LEVEL, IID_PPV_ARGS(&m_device)) );
	//���� ����̽��� �������� ������ Ȯ�ο����� �����ִ�
	//D3D12CreateDevice(hardwareAdapter.Get(), gD3D_FEATURE_LEVEL, _uuidof(ID3D12Device), nullptr);
	



	////////////////////////////////////////////////////////////////////////////////////////////////
	// Ŀ���Ŧ ���
	////////////////////////////////////////////////////////////////////////////////////////////////
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed( m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)) );




	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ü���� ��������� �����ϰ� ����.
	////////////////////////////////////////////////////////////////////////////////////////////////
	static const UINT FrameCount = 2;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));




	////////////////////////////////////////////////////////////////////////////////////////////////
	// Ǯ��ũ�� ���� ��Ʈ + ���� ����
	////////////////////////////////////////////////////////////////////////////////////////////////
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));




	////////////////////////////////////////////////////////////////////////////////////////////////
	// IDXGISwapChain1�� IDXGISwapChain3���� ����ȯ�Ѵ�
	////////////////////////////////////////////////////////////////////////////////////////////////
	ThrowIfFailed(swapChain.As(&m_swapChain));




	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ����۰� ������� �ε����� �޴´�
	// ���� �ĸ� ������ �Ǹ��� ��� ���۸� Ž��
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();




	////////////////////////////////////////////////////////////////////////////////////////////////
	// ����ü(Descriptor)��� �߻����� �������� �����ϸ�
	// �̴� ���̴� �ڿ� ����(Shader resource view) � ���Ե˴ϴ�.
	// �̷��� Descriptor�� ȿ������ ������ ���Ͽ� ����ü ����(Descriptor heap)�̶�� ���� �� �����˴ϴ�.
	// Describe and create a render target view (RTV) descriptor heap.
	////////////////////////////////////////////////////////////////////////////////////////////////
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ��ġ�� ���÷� ������ ũ�⸦ �����ɴϴ�.
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	int iiiiiii = 0;
}