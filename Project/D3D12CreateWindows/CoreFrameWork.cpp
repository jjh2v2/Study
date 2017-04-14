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




	// Create a command allocator.
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	// Create a basic command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), NULL, IID_PPV_ARGS(&m_commandList)));

	// GPU ����ȭ������ ��Ÿ���� ����ϴ�.
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_fence));

	// ��Ÿ���� ���� �̺�Ʈ ��ü�� ����ϴ�.
	m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);

	// ���� �潺 ���� �ʱ�ȭ�մϴ�.
	m_fenceValue = 1;





	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ü���� ��������� �����ϰ� ����.
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
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ��ġ�� ���÷� ������ ũ�⸦ �����ɴϴ�.
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);




	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���� ���ٽ� �� (DSV) ����� ���� ����ϰ� �����մϴ�.
	// �� �����ӿ��� ��ü ���� ���ٽ��� �ֽ��ϴ� (�׸��ڸ� ���ϴ�)
	// �׷��� ��� ��ü�� �ϳ��� �ֽ��ϴ�.
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




	// ó������ ���ڵ� ���¿��� ���� �� �� �ʱ�ȭ�ϴ� ���� ��� ����� �ݾƾ��մϴ�.
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