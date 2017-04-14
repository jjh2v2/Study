#include "stdafx.h"
#include "CoreFrameWork.h"

void CoreFrameWork::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L": " + text;
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}

// Direct3D 12를 지원하는 첫 번째 사용 가능한 하드웨어 어댑터를 얻기위한 도우미 함수.
// 그런 어댑터가 없으면 * ppAdapter는 nullptr로 설정됩니다.
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
			// 기본 렌더링 드라이버 어댑터를 선택하지 마십시오.
			// 소프트웨어 어댑터가 필요한 경우 명령 줄에서 "/ warp"를 전달하십시오.
			continue;
		}

		// 어댑터가 Direct3D 12를 지원하는지 확인하지만
		// 실제 장치는 아직 없습니다.
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
	/* DX12 다바이스 생선 파이프라인 */
	/*****************************/




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 디버그 레이어 설정
	////////////////////////////////////////////////////////////////////////////////////////////////
	INT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	// 디버그 레이어를 활성화합니다 (그래픽 도구의 "선택적 기능"필요).
	// 참고 : 장치 생성 후 디버그 레이어를 활성화하면 활성 장치가 무효화됩니다.
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		// 추가 디버그 레이어를 사용합니다.
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 펙토리 생성
	////////////////////////////////////////////////////////////////////////////////////////////////
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed( CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)) );




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 하드웨어 어뎁터 얻기
	////////////////////////////////////////////////////////////////////////////////////////////////
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	DXGI_ADAPTER_DESC1 desc;
	ThrowIfFailed(factory->EnumAdapters1(0, &hardwareAdapter));
	hardwareAdapter->GetDesc1(&desc);
	



	////////////////////////////////////////////////////////////////////////////////////////////////
	// 하드웨어 디바이스 얻기
	////////////////////////////////////////////////////////////////////////////////////////////////
	ThrowIfFailed( D3D12CreateDevice(hardwareAdapter.Get(), gD3D_FEATURE_LEVEL, IID_PPV_ARGS(&m_device)) );
	//실제 디바이스를 만들지는 않지만 확인용으로 쓸수있다
	//D3D12CreateDevice(hardwareAdapter.Get(), gD3D_FEATURE_LEVEL, _uuidof(ID3D12Device), nullptr);
	



	////////////////////////////////////////////////////////////////////////////////////////////////
	// 커멘드큔 얻기
	////////////////////////////////////////////////////////////////////////////////////////////////
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed( m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)) );




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 스왑 체인을 어떤종류인지 설명하고 생성.
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
	// 풀스크린 막기 알트 + 엔터 막기
	////////////////////////////////////////////////////////////////////////////////////////////////
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));




	////////////////////////////////////////////////////////////////////////////////////////////////
	// IDXGISwapChain1을 IDXGISwapChain3으로 형변환한단
	////////////////////////////////////////////////////////////////////////////////////////////////
	ThrowIfFailed(swapChain.As(&m_swapChain));




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 현제 백버퍼가 어떤놈인지 인덱스를 받는다
	// 현재 후면 버퍼의 권리를 얻는 버퍼를 탐색
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 정보체(Descriptor)라는 추상적인 개념으로 관리하며
	// 이는 셰이더 자원 정의(Shader resource view) 등도 포함됩니다.
	// 이러한 Descriptor는 효율적인 관리를 위하여 정보체 공간(Descriptor heap)이라는 곳에 모여 관리됩니다.
	// Describe and create a render target view (RTV) descriptor heap.
	////////////////////////////////////////////////////////////////////////////////////////////////
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 현재 장치의 샘플러 설명자 크기를 가져옵니다.
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	int iiiiiii = 0;
}