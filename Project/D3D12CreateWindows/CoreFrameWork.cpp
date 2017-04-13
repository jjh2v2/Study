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
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 하드웨어 어뎁터 얻기 시작
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
		// 정상적인 디바이스가 만들어진다 S_FALSE=0  S_OK=1 인제 디바이스가 만들어지지 않는다면 0보다 작은 값이 나온다
	}

	int iiiiiii = 0;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter0);
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

void CoreFrameWork::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L": " + text;
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}