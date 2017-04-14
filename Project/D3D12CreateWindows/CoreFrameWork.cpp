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




	// Create a command allocator.
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	// Create a basic command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), NULL, IID_PPV_ARGS(&m_commandList)));

	// GPU 동기화를위한 울타리를 만듭니다.
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_fence));

	// 울타리에 대한 이벤트 객체를 만듭니다.
	m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);

	// 시작 펜스 값을 초기화합니다.
	m_fenceValue = 1;





	////////////////////////////////////////////////////////////////////////////////////////////////
	// 스왑 체인을 어떤종류인지 설명하고 생성.
	////////////////////////////////////////////////////////////////////////////////////////////////
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
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 현재 장치의 샘플러 설명자 크기를 가져옵니다.
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);




	////////////////////////////////////////////////////////////////////////////////////////////////
	// 깊이 스텐실 뷰 (DSV) 기술자 힙을 기술하고 생성합니다.
	// 각 프레임에는 자체 깊이 스텐실이 있습니다 (그림자를 씁니다)
	// 그러면 장면 자체에 하나가 있습니다.
	////////////////////////////////////////////////////////////////////////////////////////////////
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));




	////////////////////////////////////////////////////////////////////////////////////////////////
	// Create frame resources.
	// Create a RTV for each frame.
	////////////////////////////////////////////////////////////////////////////////////////////////
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}




	////////////////////////////////////////////////////////////////////////////////////////////////
	// Create frame resources.
	// Create the depth/stencil buffer and view.
	////////////////////////////////////////////////////////////////////////////////////////////////
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// Transition the resource from its initial state to be used as a depth buffer.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));



	
	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(m_width);
	mScreenViewport.Height = static_cast<float>(m_height);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;
	mScissorRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };




	// 처음에는 레코딩 상태에서 생성 될 때 초기화하는 동안 명령 목록을 닫아야합니다.
	ThrowIfFailed(m_commandList->Close());

	int iiiiiii = 0;
}

void CoreFrameWork::OnRender()
{
	D3D12_RESOURCE_BARRIER barrier;
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
	unsigned int renderTargetViewDescriptorSize;
	float color[4];
	ID3D12CommandList* ppCommandLists[1];
	UINT64 fenceToWaitFor;


	// Reset (re-use) the memory associated command allocator.
	ThrowIfFailed(m_commandAllocator->Reset());

	// Reset the command list, use empty pipeline state for now since there are no shaders and we are just clearing the screen.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	m_commandList->RSSetViewports(1, &mScreenViewport);
	m_commandList->RSSetScissorRects(1, &mScissorRect);

	// Record commands in the command list now.
	// Start by setting the resource barrier.
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_commandList->ResourceBarrier(1, &barrier);

	// Get the render target view handle for the current back buffer.
	renderTargetViewHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (m_frameIndex == 1)
	{
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
	}

	// Set the back buffer as the render target.
	m_commandList->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, NULL);

	// Then set the color to clear the window to.
	color[0] = 0.1f;
	color[1] = 0.1f;
	color[2] = 0.5f;
	color[3] = 1.0f;
	m_commandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, NULL);

	// Indicate that the back buffer will now be used to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_commandList->ResourceBarrier(1, &barrier);

	// Close the list of commands.
	ThrowIfFailed(m_commandList->Close());

	// Load the command list array (only one command list for now).
	ppCommandLists[0] = m_commandList.Get();

	// Execute the list of commands.
	m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

	ThrowIfFailed(m_swapChain->Present(0, 0));

	// Signal and increment the fence value.
	fenceToWaitFor = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor));
	m_fenceValue++;

	// Wait until the GPU is done rendering.
	if (m_fence->GetCompletedValue() < fenceToWaitFor)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	// Alternate the back buffer index back and forth between 0 and 1 each frame.
	m_frameIndex == 0 ? m_frameIndex = 1 : m_frameIndex = 0;

}