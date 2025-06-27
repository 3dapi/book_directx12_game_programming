#pragma once
#ifndef __G2_EngineD3D_H__
#define __G2_EngineD3D_H__

#include <any>
#include <algorithm>
#include <cassert>
#include <memory>
#include <Windows.h>
#include <winerror.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl.h>
#include "include/d3dx12/d3dx12.h"
#include "G2.ConstantsWin.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace G2 {

class EngineD3D : public IG2GraphicsD3D
{
public:
	EG2GRAPHICS type() const override;
	int			init(const std::any& initialValue = {})			override;
	int			destroy()										override;
	std::any	getAttrib(int nAttrib)							override;
	int			setAttrib(int nAttrib, const std::any& v = {})	override;
	int			command(int nCmd, const std::any& v = {})		override;
	std::any	getDevice()						override;
	std::any	getRootSignature()				override;
	std::any	getCommandAllocator()			override;
	std::any	getCommandQueue()				override;
	std::any	getCommandList()				override;
	std::any	getBackBufferView()				override;
	std::any	getDepthStencilView()			override;
	std::any	getFence()						override;
	int			getCurrentBackbufferdex() const	override;
	std::any	getCurrentBackBuffer()			override;

public:
	int		InitDevice();
	int		CreateDevice();
	int		ReleaseDevice();
	void	CreateCommandObjects();
	void	CreateSwapChain();
	void	CreateRtvAndDsvDescriptorHeaps();
	int		Resize();
	int		FlushCommandQueue();
	int		FenceWait();
	int		Present();

	ID3D12Resource* CurrentBackBuffer()const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	int  Set4xMsaaState(bool msst);
	IDXGIAdapter* GetHardwareAdapter(IDXGIFactory1* pFactory, bool requestHighPerformanceAdapter = false);

protected:
	HWND        m_hWnd				{};
	::SIZE      m_screenSize		{1280, 600};
	// Set true to use 4X MSAA (?.1.8).  The default is false.
	bool        m_msaa4State		{};    // 4X MSAA enabled
	UINT        m_msaa4Quality		{};      // quality level of 4X MSAA

	// Derived class should set these in derived constructor to customize starting values.
	DXGI_FORMAT                         m_d3dFormatBackbuffer   { DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT                         m_d3dFormatDepthStencil { DXGI_FORMAT_D24_UNORM_S8_UINT };

	ComPtr<IDXGIFactory4>               m_dxgiFactory           {};
	D3D_FEATURE_LEVEL                   m_featureLevel          {};
	D3D_DRIVER_TYPE                     m_driverType            {};
	ComPtr<ID3D12Device>                m_d3dDevice             {};
	ComPtr<IDXGISwapChain>              m_d3dSwapChain          {};
	ComPtr<ID3D12Fence>                 m_d3dFence              {};
	UINT64                              m_d3dFenceIndex         {};
	UINT64                              m_d3dFenceCurrent       {};

	ComPtr<ID3D12CommandQueue>          m_d3dCommandQueue       {};
	ComPtr<ID3D12CommandAllocator>      m_d3dCommandAlloc       {};
	ComPtr<ID3D12GraphicsCommandList>   m_d3dCommandList        {};

	ComPtr<ID3D12Resource>              m_d3dBackBuffer         [FRAME_BUFFER_COUNT]{};
	ComPtr<ID3D12Resource>              m_d3dDepthBuffer        {};
	UINT                                m_d3dIndexBackBuffer    {};

	ComPtr<ID3D12DescriptorHeap>        m_heapBackBuffer        {};
	ComPtr<ID3D12DescriptorHeap>        m_heapDepthStencil      {};

	D3D12_VIEWPORT                      m_d3dViewport           {};
	D3D12_RECT                          m_d3dScissor            {};
	UINT                                m_sizeDescriptorB       {};	// back buffer
	UINT                                m_sizeDescriptorD       {};	// depth stencil
	UINT                                mCbvSrvUavDescriptorSize{};
};

}// namespace G2

#endif // __G2_EngineD3D_H__
