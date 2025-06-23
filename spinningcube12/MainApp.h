#pragma once

#include <d3dcompiler.h>
#include "D3DDevice.h"
#include "StepTimer.h"

using namespace DirectX;
using namespace Microsoft::WRL;

template<typename T>
inline void SAFE_RELEASE(T*& p)
{
	if (p)
	{
		p->Release();
		p = {};
	}
}

struct ConstBufTM
{
	XMFLOAT4X4 m{};
	static const unsigned ALIGNED_SIZE;
};

struct ConstBufMVP
{
	XMFLOAT4X4 m{};
	XMFLOAT4X4 v{};
	XMFLOAT4X4 p{};
	static const unsigned ALIGNED_SIZE;
};

struct CONST_BUF
{
	ConstBufMVP				buf {};
	ComPtr<ID3D12Resource>	rsc	{};
	uint8_t*				ptr	{};
};


struct Vertex
{
	XMFLOAT3 p;
	uint8_t  d[4];
};

class MainApp
{
protected:
	std::shared_ptr<DX::D3DDevice>						m_pDevice;
	UINT												m_d3dDescriptorSize{};


	// Direct3D resources for cube geometry.
	ComPtr<ID3D12GraphicsCommandList>		m_commandList;
	ComPtr<ID3D12DescriptorHeap>			m_cbvHeap;
	ComPtr<ID3D12RootSignature>				m_rootSignature;
	ComPtr<ID3D12PipelineState>				m_pipelineState;
	ComPtr<ID3D12Resource>					m_rscVtx;
	ComPtr<ID3D12Resource>					m_rscIdx;

	CONST_BUF								m_mvp	{};
	D3D12_VERTEX_BUFFER_VIEW				m_viewVtx;
	D3D12_INDEX_BUFFER_VIEW					m_viewIdx;

	// Variables used with the rendering loop.
	bool	m_loadingComplete;
	float	m_radiansPerSecond;
	float	m_angle;
	bool	m_tracking; 

public:
	MainApp(const std::shared_ptr<DX::D3DDevice>& deviceResources);
	~MainApp();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void Update(DX::StepTimer const& timer);
	bool Render();

protected:
	void Rotate(float radians);

	std::wstring StringToWString(const std::string& str);
	HRESULT		DXCompileShaderFromFile(const std::string& szFileName, const std::string& szEntryPoint, const std::string& szShaderModel, ID3DBlob** ppBlobOut = nullptr);
};


