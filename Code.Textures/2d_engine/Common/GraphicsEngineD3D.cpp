//***************************************************************************************
// d3dApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <memory>
#include <Windows.h>
#include "GraphicsEngineD3D.h"
#include "d3dUtil.h"
using std::unique_ptr;

static unique_ptr<EngineD3D> g_engineD3D;

IG2Graphics* IG2Graphics::instance()
{
	if (g_engineD3D)
	{
		return g_engineD3D.get();
	}
	g_engineD3D = std::make_unique<EngineD3D>();
	return g_engineD3D.get();
}

EG2GRAPHICS EngineD3D::type() const
{
	return EG2GRAPHICS::EG2_D3D12;
}

std::any EngineD3D::getAttrib(int nAttrib)
{
	switch (nCmd)
	{
		case EG2GRAPHICS_D3D::ATT_DEVICE:						return m_d3dDevice;
		case EG2GRAPHICS_D3D::ATT_SCREEN_SIZE:					return &m_screenSize;
		case EG2GRAPHICS_D3D::ATT_DEVICE_BACKBUFFER_FORAT:		return &m_d3dFormatBackbuffer;
		case EG2GRAPHICS_D3D::ATT_DEVICE_DEPTH_STENCIL_FORAT:	return &m_d3dFormatDepthStencil;
		case EG2GRAPHICS_D3D::ATT_DEVICE_CURRENT_FRAME_INDEX:	return &m_d3dCurrentBackBufferIndex;

		case EG2GRAPHICS_D3D::ATT_DEVICE_VIEWPORT:				return &m_d3dViewport;
		case EG2GRAPHICS_D3D::ATT_DEVICE_SCISSOR_RECT:			return &m_d3dScissor;
	}
	return nullptr;
	return std::any();
}

int EngineD3D::setAttrib(int nAttrib, const std::any& v)
{
	return E_FAIL;
}

int EngineD3D::command(int nCmd, const std::any& v)
{
	switch ((EG2GRAPHICS_D3D)nCmd)
	{
		case EG2GRAPHICS_D3D::CMD_MSAASTATE4X:
		{
			if (!v.has_value())
				return E_FAIL;
			auto msst = std::any_cast<bool>(v);
			return this->Set4xMsaaState(msst);
		}
		case EG2GRAPHICS_D3D::CMD_SCREEN_RESIZE:
		{
			if (!v.has_value())
				return E_FAIL;
			m_screenSize = std::any_cast<::SIZE>(v);
			return this->Resize();
		}
		default:
			break;
	}

	return E_FAIL;
}

std::any EngineD3D::getDevice()
{
	return m_d3dDevice.Get();
}

std::any EngineD3D::getRootSignature()
{
	return std::any();
}
std::any EngineD3D::getCommandAllocator()
{
	return std::any();
}
std::any EngineD3D::getCommandQueue()
{
	return std::any();
}
std::any EngineD3D::getCommandList()
{
	return m_d3dCommandList.Get();
}
std::any EngineD3D::getRenderTarget()
{
	return std::any();
}
std::any EngineD3D::getRenderTargetView()
{
	return std::any();
}
std::any EngineD3D::getDepthStencilView()
{
	return std::any();
}
int  EngineD3D::getCurrentFrameIndex()	const
{
	return m_d3dCurrentBackBufferIndex;
}

int EngineD3D::init(const std::any& initialValue)
{
	if (!initialValue.has_value())
		return E_FAIL;
	auto [hWnd, size, m4xmssa] = std::any_cast<std::tuple<HWND, ::SIZE, bool> >(initialValue);
	m_hWnd = hWnd;
	m_screenSize = size;
	m4xMsaaState = m4xmssa;
	return 0;
}


bool EngineD3D::InitDirect3D()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_d3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_d3dDevice)));
	}

	ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));

	mRtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_d3dFormatBackbuffer;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	return true;
}

void EngineD3D::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_d3dCommandQueue)));
	ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_d3dCommandAlloc.GetAddressOf())));
	ThrowIfFailed(m_d3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_d3dCommandAlloc.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(m_d3dCommandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	m_d3dCommandList->Close();
}

void EngineD3D::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	m_d3dSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = m_screenSize.cx;
	sd.BufferDesc.Height = m_screenSize.cy;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_d3dFormatBackbuffer;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = FRAME_BUFFER_COUNT;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
		m_d3dCommandQueue.Get(),
		&sd,
		m_d3dSwapChain.GetAddressOf()));
}

void EngineD3D::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

int EngineD3D::Resize()
{
	if (!m_d3dDevice || !m_d3dSwapChain || !m_d3dCommandAlloc)
		return E_FAIL;

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(m_d3dCommandList->Reset(m_d3dCommandAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
		m_d3dFrameBufferRenderTarget[i].Reset();
	m_d3dFrameBufferDepthStencil.Reset();

	// Resize the swap chain.
	ThrowIfFailed(m_d3dSwapChain->ResizeBuffers(
		FRAME_BUFFER_COUNT,
		m_screenSize.cx, m_screenSize.cy,
		m_d3dFormatBackbuffer,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_d3dCurrentBackBufferIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		ThrowIfFailed(m_d3dSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_d3dFrameBufferRenderTarget[i])));
		m_d3dDevice->CreateRenderTargetView(m_d3dFrameBufferRenderTarget[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC descDepth;
	descDepth.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	descDepth.Alignment = 0;
	descDepth.Width = m_screenSize.cx;
	descDepth.Height = m_screenSize.cy;
	descDepth.DepthOrArraySize = 1;
	descDepth.MipLevels = 1;
	descDepth.Format = m_d3dFormatDepthStencil;
	descDepth.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	descDepth.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	descDepth.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	descDepth.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_d3dFormatDepthStencil;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&descDepth,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_d3dFrameBufferDepthStencil.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	m_d3dDevice->CreateDepthStencilView(m_d3dFrameBufferDepthStencil.Get(), nullptr, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	m_d3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_d3dFrameBufferDepthStencil.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(m_d3dCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_d3dCommandList.Get() };
	m_d3dCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = static_cast<float>(m_screenSize.cx);
	m_d3dViewport.Height = static_cast<float>(m_screenSize.cy);
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	m_d3dScissor = { 0, 0, m_screenSize.cx, m_screenSize.cy };

	return 1;
}


void EngineD3D::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	mCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(m_d3dCommandQueue->Signal(mFence.Get(), mCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource* EngineD3D::CurrentBackBuffer()const
{
	return m_d3dFrameBufferRenderTarget[m_d3dCurrentBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE EngineD3D::CurrentBackBufferView()const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_d3dCurrentBackBufferIndex,
		mRtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE EngineD3D::DepthStencilView()const
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void EngineD3D::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		_SAFE_RELEASE_(adapterList[i]);
	}
}

void EngineD3D::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, m_d3dFormatBackbuffer);

		_SAFE_RELEASE_(output);

		++i;
	}
}

void EngineD3D::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

int EngineD3D::Set4xMsaaState(bool msst)
{
	if (m4xMsaaState == msst)
		return E_FAIL;

	m4xMsaaState = msst;
	this->CreateSwapChain();
	int hr = this->Resize();
	return hr;
}
