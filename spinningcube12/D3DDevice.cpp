#include "pch.h"
#include "D3DDevice.h"
#include "DirectXHelper.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace DisplayMetrics
{
	// High resolution displays can require a lot of GPU and battery power to render.
	// High resolution phones, for example, may suffer from poor battery life if
	// games attempt to render at 60 frames per second at full fidelity.
	// The decision to render at full fidelity across all platforms and form factors
	// should be deliberate.
	static const bool SupportHighResolutions = false;

	// The default thresholds that define a "high resolution" display. If the thresholds
	// are exceeded and SupportHighResolutions is false, the dimensions will be scaled
	// by 50%.
	static const float DpiThreshold = 192.0f;		// 200% of standard desktop display.
	static const float WidthThreshold = 1920.0f;	// 1080p width.
	static const float HeightThreshold = 1080.0f;	// 1080p height.
};

// Constants used to calculate screen rotations.
namespace ScreenRotation
{
	// 0-degree Z-rotation
	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// 90-degree Z-rotation
	static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// 180-degree Z-rotation
	static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// 270-degree Z-rotation
	static const XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
};

// Constructor for DeviceResources.
DX::D3DDevice::D3DDevice(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat) :
	m_d3dCurrentFrameIndex(0),
	m_d3dViewport(),
	m_d3dSizeDescRenderTarget(0),
	m_fenceEvent(0),
	m_d3dFormatBackBuffer(backBufferFormat),
	m_formatDepthBuffer(depthBufferFormat),
	m_fenceValue{},
	m_d3dRenderTargetSize(),
	m_d3dOutputSize(),
	m_logicalSize(),
	m_dpi(96.0f),
	m_effectiveDpi(-1.0f),
	m_deviceRemoved(false)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();
}

// Configures resources that don't depend on the Direct3D device.
void DX::D3DDevice::CreateDeviceIndependentResources()
{
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DX::D3DDevice::CreateDeviceResources()
{
#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(&adapter);

	// Create the Direct3D 12 API device object
	HRESULT hr = D3D12CreateDevice(
		adapter.Get(),					// The hardware adapter.
		D3D_FEATURE_LEVEL_11_0,			// Minimum feature level this app can support.
		IID_PPV_ARGS(&m_d3dDevice)		// Returns the Direct3D device created.
		);

#if defined(_DEBUG)
	if (FAILED(hr))
	{
		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// https://go.microsoft.com/fwlink/?LinkId=286690

		ComPtr<IDXGIAdapter> warpAdapter;
		DX::ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));
	}
#endif

	DX::ThrowIfFailed(hr);

	// Create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_d3dCommandQueue)));

	m_d3dSizeDescRenderTarget = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create descriptor heaps for render target views and depth stencil views.
	D3D12_DESCRIPTOR_HEAP_DESC descRenderTargetHeap {};
	descRenderTargetHeap.NumDescriptors = FRAME_BUFFER_COUNT;
	descRenderTargetHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descRenderTargetHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&descRenderTargetHeap, IID_PPV_ARGS(&m_d3dDescTarget)));

	D3D12_DESCRIPTOR_HEAP_DESC descDepthStencilHeap {};
	descDepthStencilHeap.NumDescriptors = 1;
	descDepthStencilHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descDepthStencilHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&descDepthStencilHeap, IID_PPV_ARGS(&m_d3dDescDepth)));

	for (UINT n = 0; n < FRAME_BUFFER_COUNT; n++)
	{
		DX::ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_d3dCommandAllocator[n])) );
	}

	// Create synchronization objects.
	DX::ThrowIfFailed(m_d3dDevice->CreateFence(m_fenceValue[m_d3dCurrentFrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue[m_d3dCurrentFrameIndex]++;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

// These resources need to be recreated every time the window size is changed.
void DX::D3DDevice::CreateWindowSizeDependentResources()
{
	// Wait until all previous GPU work is complete.
	WaitForGpu();

	// Clear the previous window size specific content and update the tracked fence values.
	for (UINT n = 0; n < FRAME_BUFFER_COUNT; n++)
	{
		m_d3dRenderTarget[n] = nullptr;
		m_fenceValue[n] = m_fenceValue[m_d3dCurrentFrameIndex];
	}

	RECT rect{};
	GetClientRect(m_window, &rect);
	m_logicalSize.Width = static_cast<float>(rect.right - rect.left);
	m_logicalSize.Height = static_cast<float>(rect.bottom - rect.top);

	UpdateRenderTargetSize();

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = DXGI_MODE_ROTATION_UNSPECIFIED;
	m_d3dRenderTargetSize.Width = m_d3dOutputSize.Width;
	m_d3dRenderTargetSize.Height = m_d3dOutputSize.Height;

	UINT backBufferWidth = lround(m_d3dRenderTargetSize.Width);
	UINT backBufferHeight = lround(m_d3dRenderTargetSize.Height);

	if (m_d3dSwapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_d3dSwapChain->ResizeBuffers(FRAME_BUFFER_COUNT, backBufferWidth, backBufferHeight, m_d3dFormatBackBuffer, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			m_deviceRemoved = true;

			// Do not continue execution of this method. DeviceResources will be destroyed and re-created.
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 descSwap = {};
			descSwap.Width = backBufferWidth;						// Match the size of the window.
			descSwap.Height = backBufferHeight;
			descSwap.Format = m_d3dFormatBackBuffer;
			descSwap.Stereo = false;
			descSwap.SampleDesc.Count = 1;							// Don't use multi-sampling.
			descSwap.SampleDesc.Quality = 0;
			descSwap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			descSwap.BufferCount = FRAME_BUFFER_COUNT;					// Use triple-buffering to minimize latency.
			descSwap.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			descSwap.Flags = 0;
			descSwap.Scaling = scaling;
			descSwap.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		DX::ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
			m_d3dCommandQueue.Get(),
			m_window,
			&descSwap,
			nullptr,
			nullptr,
			(IDXGISwapChain1**)m_d3dSwapChain.GetAddressOf()
		)
		);
	}

	// Create render target views of the swap chain back buffer.
	{
		m_d3dCurrentFrameIndex = m_d3dSwapChain->GetCurrentBackBufferIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_d3dDescTarget->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FRAME_BUFFER_COUNT; n++)
		{
			DX::ThrowIfFailed(m_d3dSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_d3dRenderTarget[n])));
			m_d3dDevice->CreateRenderTargetView(m_d3dRenderTarget[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(m_d3dSizeDescRenderTarget);

			WCHAR name[128]{};
			if (swprintf_s(name, L"m_renderTargets[%u]", n) > 0)
			{
				DX::SetName(m_d3dRenderTarget[n].Get(), name);
			}
		}
	}

	HRESULT hr = S_OK;
	// 7. Create a depth stencil and view.
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_formatDepthBuffer;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_formatDepthBuffer, backBufferWidth, backBufferHeight, 1, 1);
		depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		CD3DX12_CLEAR_VALUE depthOptimizedClearValue(m_formatDepthBuffer, 1.0f, 0);

		hr = m_d3dDevice->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_d3dRenderDepth)
			);
		if (FAILED(hr))
			return;

		m_d3dDevice->CreateDepthStencilView(m_d3dRenderDepth.Get(), &dsvDesc, m_d3dDescDepth->GetCPUDescriptorHandleForHeapStart());
	}

	// Set the 3D rendering viewport to target the entire window.
	m_d3dViewport = { 0.0f, 0.0f, static_cast<float>(m_d3dRenderTargetSize.Width), static_cast<float>(m_d3dRenderTargetSize.Height), 0.0f, 1.0f };
	m_scissorRect = { 0,    0   , static_cast<LONG>(m_d3dRenderTargetSize.Width), static_cast<LONG>(m_d3dRenderTargetSize.Height)};
}

// Determine the dimensions of the render target and whether it will be scaled down.
void DX::D3DDevice::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;

	// To improve battery life on high resolution devices, render to a smaller render target
	// and allow the GPU to scale the output when it is presented.
	if (!DisplayMetrics::SupportHighResolutions && m_dpi > DisplayMetrics::DpiThreshold)
	{
		float width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_dpi);
		float height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_dpi);

		// When the device is in portrait orientation, height > width. Compare the
		// larger dimension against the width threshold and the smaller dimension
		// against the height threshold.
		if (max(width, height) > DisplayMetrics::WidthThreshold && min(width, height) > DisplayMetrics::HeightThreshold)
		{
			// To scale the app we change the effective DPI. Logical size does not change.
			m_effectiveDpi /= 2.0f;
		}
	}

	// Calculate the necessary render target size in pixels.
	m_d3dOutputSize.Width = static_cast<unsigned int>(DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi));
	m_d3dOutputSize.Height = static_cast<unsigned int>(DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi));

	// Prevent zero size DirectX content from being created.
	m_d3dOutputSize.Width = max(m_d3dOutputSize.Width, 1);
	m_d3dOutputSize.Height = max(m_d3dOutputSize.Height, 1);
}

// This method is called when the CoreWindow is created (or re-created).
void DX::D3DDevice::SetWindow(HWND hwnd)
{
	m_window = hwnd;

	CreateWindowSizeDependentResources();
}

// This method is called in the event handler for the SizeChanged event.
void DX::D3DDevice::SetLogicalSize(SizeF logicalSize)
{
	if (static_cast<int>(m_logicalSize.Width) != static_cast<int>(logicalSize.Width) ||
		static_cast<int>(m_logicalSize.Height) != static_cast<int>(logicalSize.Height))
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void DX::D3DDevice::ValidateDevice()
{
	// The D3D Device is no longer valid if the default adapter changed since the device
	// was created or if the device has been removed.

	// First, get the LUID for the default adapter from when the device was created.

	DXGI_ADAPTER_DESC previousDesc;
	{
		ComPtr<IDXGIAdapter1> previousDefaultAdapter;
		DX::ThrowIfFailed(m_dxgiFactory->EnumAdapters1(0, &previousDefaultAdapter));

		DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));
	}

	// Next, get the information for the current default adapter.

	DXGI_ADAPTER_DESC currentDesc;
	{
		ComPtr<IDXGIFactory4> currentDxgiFactory;
		DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentDxgiFactory)));

		ComPtr<IDXGIAdapter1> currentDefaultAdapter;
		DX::ThrowIfFailed(currentDxgiFactory->EnumAdapters1(0, &currentDefaultAdapter));

		DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));
	}

	// If the adapter LUIDs don't match, or if the device reports that it has been removed,
	// a new D3D device must be created.

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		m_deviceRemoved = true;
	}
}

// Present the contents of the swap chain to the screen.
void DX::D3DDevice::Present()
{
	HRESULT hr = m_d3dSwapChain->Present(1, 0);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		m_deviceRemoved = true;
	}
	else
	{
		DX::ThrowIfFailed(hr);
		MoveToNextFrame();
	}
}

// Wait for pending GPU work to complete.
void DX::D3DDevice::WaitForGpu()
{
	DX::ThrowIfFailed(m_d3dCommandQueue->Signal(m_fence.Get(), m_fenceValue[m_d3dCurrentFrameIndex]));
	DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue[m_d3dCurrentFrameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	m_fenceValue[m_d3dCurrentFrameIndex]++;
}

// Prepare to render the next frame.
void DX::D3DDevice::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValue[m_d3dCurrentFrameIndex];
	DX::ThrowIfFailed(m_d3dCommandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Advance the frame index.
	m_d3dCurrentFrameIndex = m_d3dSwapChain->GetCurrentBackBufferIndex();

	// Check to see if the next frame is ready to start.
	if (m_fence->GetCompletedValue() < m_fenceValue[m_d3dCurrentFrameIndex])
	{
		DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue[m_d3dCurrentFrameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValue[m_d3dCurrentFrameIndex] = currentFenceValue + 1;
}

// This method acquires the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
void DX::D3DDevice::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
