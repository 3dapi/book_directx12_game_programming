#pragma once

using Microsoft::WRL::ComPtr;

namespace DX
{
	static const UINT FRAME_BUFFER_COUNT = 2;		// Use triple buffering.

	struct SizeF
	{
		float Width;
		float Height;
	};
	struct SizeI
	{
		int Width;
		int Height;
	};
	struct SizeU
	{
		unsigned int Width;
		unsigned int Height;
	};

	// Controls all the DirectX device resources.
	class D3DDevice
	{
	public:
		D3DDevice(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT);
		void SetWindow(HWND window);
		void SetLogicalSize(SizeF logicalSize);
		void ValidateDevice();
		void Present();
		void WaitForGpu();

		// The size of the render target, in pixels.
		SizeU						GetOutputSize() const				{ return m_d3dOutputSize; }

		// The size of the render target, in dips.
		SizeF						GetLogicalSize() const				{ return m_logicalSize; }

		float						GetDpi() const						{ return m_effectiveDpi; }
		bool						IsDeviceRemoved() const				{ return m_deviceRemoved; }

		// D3D Accessors.
		ID3D12Device*				GetD3DDevice() const				{ return m_d3dDevice.Get(); }
		IDXGISwapChain3*			GetSwapChain() const				{ return m_d3dSwapChain.Get(); }
		ID3D12Resource*				GetRenderTarget() const				{ return m_d3dRenderTarget[m_d3dCurrentFrameIndex].Get(); }
		ID3D12Resource*				GetDepthStencil() const				{ return m_d3dRenderDepth.Get(); }
		ID3D12CommandQueue*			GetCommandQueue() const				{ return m_d3dCommandQueue.Get(); }
		ID3D12CommandAllocator*		GetCommandAllocator() const			{ return m_d3dCommandAllocator[m_d3dCurrentFrameIndex].Get(); }
		DXGI_FORMAT					GetBackBufferFormat() const			{ return m_d3dFormatBackBuffer; }
		DXGI_FORMAT					GetDepthBufferFormat() const		{ return m_formatDepthBuffer; }
		D3D12_VIEWPORT				GetScreenViewport() const			{ return m_d3dViewport; }
		D3D12_RECT					GetScreenScissorRect() const		{ return m_scissorRect; }
		DirectX::XMFLOAT4X4			GetOrientationTransform3D() const	{ return m_d3dOrientationTransform3D; }
		UINT						GetCurrentFrameIndex() const		{ return m_d3dCurrentFrameIndex; }

		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_d3dDescTarget->GetCPUDescriptorHandleForHeapStart(), m_d3dCurrentFrameIndex, m_d3dSizeDescRenderTarget);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_d3dDescDepth->GetCPUDescriptorHandleForHeapStart());
		}

	private:
		void CreateDeviceIndependentResources();
		void CreateDeviceResources();
		void CreateWindowSizeDependentResources();
		void UpdateRenderTargetSize();
		void MoveToNextFrame();
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);

		UINT											m_d3dCurrentFrameIndex;

		// Direct3D objects.
		ComPtr<ID3D12Device>				m_d3dDevice;
		ComPtr<IDXGIFactory4>				m_dxgiFactory;
		ComPtr<IDXGISwapChain3>				m_d3dSwapChain;
		ComPtr<ID3D12DescriptorHeap>		m_d3dDescTarget;
		ComPtr<ID3D12Resource>				m_d3dRenderTarget[FRAME_BUFFER_COUNT];
		ComPtr<ID3D12DescriptorHeap>		m_d3dDescDepth;
		ComPtr<ID3D12Resource>				m_d3dRenderDepth;
		ComPtr<ID3D12CommandAllocator>		m_d3dCommandAllocator[FRAME_BUFFER_COUNT];
		ComPtr<ID3D12CommandQueue>			m_d3dCommandQueue;
		DXGI_FORMAT							m_d3dFormatBackBuffer;
		DXGI_FORMAT							m_formatDepthBuffer;
		D3D12_VIEWPORT						m_d3dViewport	{};
		D3D12_RECT							m_scissorRect	{};
		UINT								m_d3dSizeDescRenderTarget;
		bool								m_deviceRemoved;

		// CPU/GPU Synchronization.
		ComPtr<ID3D12Fence>					m_fence;
		UINT64								m_fenceValue[FRAME_BUFFER_COUNT];
		HANDLE								m_fenceEvent;

		// Cached reference to the Window.
		HWND								m_window;

		// Cached device properties.
		SizeU								m_d3dRenderTargetSize;
		SizeU								m_d3dOutputSize;
		SizeF								m_logicalSize;
		float								m_dpi;

		// This is the DPI that will be reported back to the app. It takes into account whether the app supports high resolution screens or not.
		float								m_effectiveDpi;

		// Transforms used for display orientation.
		DirectX::XMFLOAT4X4					m_d3dOrientationTransform3D;
	};
}
